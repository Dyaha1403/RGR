#include "utilites/glwnd.h"
#include "utilites/camera_quat/camera.h"
#include "utilites/camera_utils.h"

//#include <winsock2.h>
#include <iostream>
#include <cstring>
#include <thread>
#pragma comment(lib, "ws2_32.lib")

INT Width, Height;
CCamera cam(45.0, { 0.0f,0.0f,0.0f });
float speed = 0.01f;

void handleClient(SOCKET clientSocket, CCamera& cam) {
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));

	while (true) {
		int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (bytesRead <= 0) {
			std::cerr << "Connection closed by the client." << std::endl;
			break;
		}

		// Extract gyroscope data from the received buffer
		double x, y, z;
		sscanf_s(buffer, "{'accelerationX': %lf, 'accelerationY': %lf, 'accelerationZ': %lf}", &x, &y, &z); // 

		// Update camera orientation based on gyroscope data
		cam.UpdateOrientationFromGyroscope(x, y, z);

		// Clear the buffer for the next data reception
		memset(buffer, 0, sizeof(buffer));
	}

	closesocket(clientSocket);
}

void startServer(CCamera& cam) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "Failed to initialize Winsock." << std::endl;
		return;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "Failed to create socket." << std::endl;
		return;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(6000);  // Specify the port you want to use
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
		std::cerr << "Failed to bind socket." << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	listen(serverSocket, 5);
	std::cout << "C++ server is listening on port 6000..." << std::endl;

	while (true) {
		SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
		std::cout << "Python client connected." << std::endl;

		// Start a new thread for handling the client
		std::thread(handleClient, clientSocket, std::ref(cam)).detach();
	}

	closesocket(serverSocket);
	WSACleanup();
}


void DrawCube()
{
	glBegin(GL_QUADS);

	// Front face
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);

	// Back face
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	// Top face
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	// Bottom face
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);

	// Right face
	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);

	// Left face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);

	glEnd();
}


void fn_draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	cam.UpdateCameraState();

	CameraUtils_LookAt(cam);
	DrawCube();

	CameraUtils_DrawCrosshair(Width, Height);

}

void fn_window_resize(HWND hWnd, int width, int height)
{
	if (!height)
		height = 1;

	Width = width;
	Height = height;
	cam.m_iScreenWidth = width;
	cam.m_iScreenHeight = height;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, width / (double)height, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void fn_mousemove(HWND hWnd, int x, int y)
{
}

void fn_mouseclick(HWND hWnd, int x, int y, int button, int state)
{
}

void fn_charinput(HWND hWnd, char symbol)
{
	
}

//https://docs.microsoft.com/ru-ru/windows/win32/inputdev/wm-keydown
void fn_keydown(HWND hWnd, INT state, WPARAM wparam, LPARAM lparam)
{
	INT key = (INT)wparam;
	CameraUtils_CheckActive(cam, GetKeyState(VK_F1), key);
	CameraUtils_Move(cam, key, state, speed);
	if (state == KEY_DOWN) {
		switch (key) {
		case 27:
			exit(0); //close program
			break;
		}
	}
}

//Add this GL functions
void fn_windowcreate(HWND hWnd)
{
	RECT rct;
	GetClientRect(hWnd, &rct);
	glViewport(0, 0, (GLsizei)rct.right, (GLsizei)rct.bottom);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, rct.right / (double)rct.bottom, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void fn_windowclose(HWND hWnd)
{
	exit(0);
}

int main()
{
	std::thread(startServer, std::ref(cam)).detach();
	create_window("Sample OpenGL Application", __FILE__ __TIME__,
		24,					  //Colors bits
		32,					  //Depth bits
		fn_draw,			  //Draw function
		fn_window_resize,	  //Window resize function
		fn_mousemove,		  //Mouse move function
		fn_mouseclick,		  //Mouse click function
		fn_charinput,		  //Char input function
		fn_keydown,			  //Keydown function
		fn_windowcreate,	  //Window create function
		fn_windowclose,		  //Window close function
		0, 0,
		800,				  //Window width
		600);				  //Window height
	return 0;
}