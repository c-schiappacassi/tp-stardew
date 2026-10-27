#include "gtest/gtest.h"
#include "socket.h"
#include "../client/client_protocol.h"
#include "../server/server_protocol.h"
#include "dtos.h"
#include "../server/game_manager.h"
#include "../server/acceptor.h"
#include <thread>
#include <chrono>

TEST(NetworkTest, CreateGameIntegrationTest) {
    const char* testPort = "12350";

    GameManager manager;
    manager.createGame(testPort);

    Socket clientSocket("127.0.0.1", testPort);
    ClientProtocol clientProtocol(clientSocket);

    CreateGameDTO createDto("Jugador1");
    createDto.gameName = "PartidaTest";
    createDto.worldId = 0;

    clientProtocol.send(createDto);

    ServerStatus status;
    clientProtocol.recv(status);

    EXPECT_EQ(status.status, StatusCode::Ok);

    manager.stopAll();
}
