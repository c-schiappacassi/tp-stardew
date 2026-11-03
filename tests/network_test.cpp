#include <chrono>
#include <memory>
#include <future>
#include <thread>

#include "gtest/gtest.h"
#include "client_protocol.h"
#include "network.h"
#include "server/game/game_manager.h"
#include "server/network/acceptor.h"

using namespace std::chrono_literals;

TEST(QueueTest, ClosingAFullQueueWakesBlockedProducer) {
    Queue<int> queue(1);
    queue.push(1);
    auto producer = std::async(std::launch::async, [&queue] {
        try {
            queue.push(2);
            return false;
        } catch (const ClosedQueue&) {
            return true;
        }
    });
    EXPECT_EQ(producer.wait_for(20ms), std::future_status::timeout);
    queue.close();
    EXPECT_EQ(producer.wait_for(1s), std::future_status::ready);
    EXPECT_TRUE(producer.get());
}

class NetworkTest : public ::testing::Test {
protected:
    GameManager manager;
    Socket listener{"0"};
    Acceptor acceptor{listener, manager};
    std::string port = std::to_string(listener.getLocalPort());

    bool acceptorJoined = false;

    void SetUp() override { acceptor.start(); }
    void TearDown() override {
        if (!acceptorJoined) {
            acceptor.stop();
            acceptor.join();
        }
        manager.stopAll();
    }

    CreateGameDTO create(const std::string& user) {
        CreateGameDTO dto(user);
        dto.gameName = "Granja de prueba";
        dto.characterName = "Personaje " + user;
        dto.hairModel = 5;
        dto.shirtModel = 2;
        dto.pantsModel = 1;
        dto.hairColor = {12, 34, 56};
        dto.pantsColor = {78, 90, 123};
        return dto;
    }
};

TEST_F(NetworkTest, MenuQueriesAndFailedEntriesKeepTheStreamAligned) {
    NetworkManager client("127.0.0.1", port);
    EXPECT_TRUE(client.handshakeQt(ListGamesDTO("ana")).games.empty());
    CheckCodeDTO code("ana");
    code.code = "inexistente";
    EXPECT_EQ(client.handshakeQt(code).status, StatusCode::Error);
    ResumeGameDTO resume("ana");
    resume.gameId = "inexistente";
    EXPECT_EQ(client.handshake(resume).status, StatusCode::Error);
    auto invalid = create("ana");
    invalid.gameName.clear();
    EXPECT_EQ(client.handshake(invalid).status, StatusCode::Error);
    EXPECT_EQ(client.handshakeQt(ListGamesDTO("ana")).status, StatusCode::Ok);

    const auto status = client.handshake(create("ana"));
    ASSERT_EQ(status.status, StatusCode::Ok);
    ASSERT_TRUE(status.players.contains("ana"));
    EXPECT_EQ(status.localPlayer.characterName, "Personaje ana");
    EXPECT_EQ(status.localPlayer.hairModel, 5);
    EXPECT_EQ(status.localPlayer.hairColor.g, 34);
    EXPECT_EQ(status.localPlayer.pantsColor.b, 123);
    EXPECT_EQ(status.localPlayer.items.size(), 4u);
    const auto& mine = std::get<MineData>(status.localPlayer.zoneData);
    EXPECT_EQ(mine.current_level, 1);
    EXPECT_FALSE(mine.current_level_data.minerals.empty());

    // This is the exact connection handed from Qt to SDL: no second handshake.
    client.start();
    auto select = std::make_shared<SelectSlotDTO>("ana");
    select->slot = 2;
    client.push_to_send(select);
    bool updated = false;
    const auto deadline = std::chrono::steady_clock::now() + 2s;
    while (std::chrono::steady_clock::now() < deadline && !updated) {
        while (auto state = client.try_pop_recv()) {
            updated = state->localPlayer.current_slot == 2;
        }
        std::this_thread::sleep_for(5ms);
    }
    EXPECT_TRUE(updated);
    client.quit();
    client.quit();
}

TEST_F(NetworkTest, JoinListDisconnectAndResumePreservePlayerState) {
    std::string gameId;
    ServerStatus original;
    {
        NetworkManager owner("127.0.0.1", port);
        original = owner.handshake(create("ana"));
        ASSERT_EQ(original.status, StatusCode::Ok);
        gameId = original.gameId;

        NetworkManager guest("127.0.0.1", port);
        CheckCodeDTO check("luis");
        check.code = gameId;
        EXPECT_EQ(guest.handshakeQt(check).status, StatusCode::Ok);
        JoinGameDTO join("luis");
        join.code = gameId;
        join.characterName = "Invitado";
        join.hairModel = 9;
        const auto joined = guest.handshake(join);
        ASSERT_EQ(joined.status, StatusCode::Ok);
        EXPECT_EQ(joined.gameId, gameId);
        EXPECT_EQ(joined.localPlayer.characterName, "Invitado");
        EXPECT_EQ(joined.localPlayer.hairModel, 9);
        EXPECT_EQ(joined.players.size(), 2u);

        NetworkManager menu("127.0.0.1", port);
        const auto listed = menu.handshakeQt(ListGamesDTO("ana"));
        ASSERT_EQ(listed.games.size(), 1u);
        EXPECT_EQ(listed.games[0].id, gameId);
        EXPECT_EQ(listed.games[0].farmName, "Granja de prueba");
        EXPECT_EQ(listed.games[0].playerData.characterName, "Personaje ana");
        EXPECT_TRUE(listed.games[0].isMultiplayer);
        EXPECT_TRUE(menu.handshakeQt(ListGamesDTO("otro")).games.empty());
        ResumeGameDTO active("ana");
        active.gameId = gameId;
        EXPECT_EQ(menu.handshake(active).status, StatusCode::Error);
    }

    NetworkManager resumed("127.0.0.1", port);
    ResumeGameDTO resume("ana");
    resume.gameId = gameId;
    ServerStatus status;
    const auto deadline = std::chrono::steady_clock::now() + 2s;
    do {
        status = resumed.handshake(resume);
        if (status.status == StatusCode::Ok) break;
        std::this_thread::sleep_for(5ms);
    } while (std::chrono::steady_clock::now() < deadline);
    ASSERT_EQ(status.status, StatusCode::Ok) << status.errorMsg;
    EXPECT_EQ(status.playerId, original.playerId);
    EXPECT_EQ(status.localPlayer.characterName, original.localPlayer.characterName);
    EXPECT_EQ(status.localPlayer.gold, original.localPlayer.gold);
    EXPECT_EQ(status.localPlayer.energy, original.localPlayer.energy);
    EXPECT_EQ(status.localPlayer.hairColor.r, original.localPlayer.hairColor.r);
}

TEST_F(NetworkTest, DirectionCommandsAndSlotRemainAligned) {
    Socket socket("127.0.0.1", port.c_str());
    ClientProtocol protocol(socket);
    protocol.send(create("ana"));
    ServerStatus status;
    protocol.recv(status);
    ASSERT_EQ(status.status, StatusCode::Ok);
    // Select the sword first so random nearby minerals cannot start a pickaxe
    // animation and temporarily reject subsequent inventory changes.
    SelectSlotDTO slot("ana");
    slot.slot = 3;
    protocol.send(slot);
    InteractDTO interact("ana");
    interact.dir = Direction::Right;
    protocol.send(interact);
    MoveDTO move("ana");
    move.dir = Direction::Right;
    protocol.send(move);
    slot.slot = 2;
    protocol.send(slot);
    bool updated = false;
    for (int i = 0; i < 60 && !updated; ++i) {
        protocol.recv(status);
        updated = status.localPlayer.current_slot == 2;
    }
    EXPECT_TRUE(updated);
    EXPECT_EQ(status.localPlayer.dir, Direction::Right);
    protocol.send(DisconnectDTO("ana"));
}

TEST_F(NetworkTest, ServerShutdownWakesClientThreadsAndBlockedReceivers) {
    NetworkManager idleMenu("127.0.0.1", port);
    NetworkManager client("127.0.0.1", port);
    ASSERT_EQ(client.handshake(create("ana")).status, StatusCode::Ok);
    client.start();
    acceptor.stop();
    acceptor.join();
    acceptorJoined = true;
    const auto deadline = std::chrono::steady_clock::now() + 2s;
    while (client.isConnected() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(5ms);
    }
    EXPECT_FALSE(client.isConnected());
    client.quit();
}

TEST_F(NetworkTest, MultipleGamesShareTheListenerAndKeepSeparatePlayers) {
    NetworkManager first("127.0.0.1", port);
    NetworkManager second("127.0.0.1", port);
    const auto firstGame = first.handshake(create("ana"));
    const auto secondGame = second.handshake(create("luis"));
    ASSERT_EQ(firstGame.status, StatusCode::Ok);
    ASSERT_EQ(secondGame.status, StatusCode::Ok);
    ASSERT_NE(firstGame.gameId, secondGame.gameId);

    NetworkManager guest("127.0.0.1", port);
    JoinGameDTO join("ana");
    join.code = secondGame.gameId;
    join.characterName = "Otro personaje";
    const auto joined = guest.handshake(join);
    ASSERT_EQ(joined.status, StatusCode::Ok);
    EXPECT_EQ(joined.gameId, secondGame.gameId);
    EXPECT_EQ(joined.players.size(), 2u);
    EXPECT_EQ(joined.localPlayer.characterName, "Otro personaje");

    NetworkManager menu("127.0.0.1", port);
    const auto listed = menu.handshakeQt(ListGamesDTO("ana"));
    ASSERT_EQ(listed.games.size(), 2u);
    EXPECT_FALSE(listed.games[0].isMultiplayer);
    EXPECT_TRUE(listed.games[1].isMultiplayer);
}
