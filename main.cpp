#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include "FundLibs/glad 46/include/glad/glad.h"
#include "FundLibs/sh_win/keys.h"
#include "FundLibs/sh_win/win.h"
#include <chrono>
#include <string>
#include <winuser.h>
#include "FundLibs/vogl/Shader.h"
#include "var_o_vogl/var.h"
#include "var_o_vogl/chat_message.hpp"
#include "FundLibs/noise/Simplex.h"
#include "var_o_vogl/var_renderer.h"
#include "var_o_vogl/mat3x3.h"
#include "var_o_vogl/vox_obj.h"
#include "var_o_vogl/chat_message.cpp"

#pragma comment(lib, "opengl32.lib")

struct ubo_buf {
	float n;
	float aspect;
	float h;

	float pos[3];
	float ang[3];

	float mod_pos[3];

	fmat3x3 mod_mat;
} ubo_buf;

pipeprog shader;

fwind gr_win;
fwind co_win;
HDC hdc;
WINDOWPLACEMENT wpc;
uint32_t cam_ubo;

uint32_t vox_buf;

HGLRC hRC;

SimplexNoise nois;

Vobj V[Vobj::max_obj_amount];
var v[Vobj::max_obj_amount];
Vobj Cam_vox;
var cam_vox;
Wobj W;
boost::asio::io_context io_context;
bool is_alive = true;

const int wwidth = 1920;
const int wheight = 1080;
float camscale = 0.02f * Vobj::obj_volume;
float speedscale = 0.15f * sqrt(Vobj::obj_volume) * 2;
float mousescale = 0.2f;
uint32_t maxmousespeed = 50;
float friction = 0.8f;
float mousex, prevmousex = wwidth/2;
float mousey, prevmousey = wheight/2;

boost::asio::ip::tcp::resolver resolver(io_context);
auto endpoints = resolver.resolve("192.168.1.64", "13");
chat_client c(io_context, endpoints);
chat_message Bytes;

std::chrono::microseconds webtime;
size_t webt_old = webtime.count();
CURSORINFO cinf;
POINT mousept;

void keysfunc() {
	static POINT lpPoint;
	while (is_alive) {
		if (!get_key(VK_MENU).held) {
			SetCursorPos(prevmousex, prevmousey);
			Sleep(1);
			GetCursorPos(&lpPoint);
			mousex = lpPoint.x;
			mousey = lpPoint.y;
			if ((mousey - prevmousey) > maxmousespeed) mousey = prevmousey + maxmousespeed;
			if ((-mousey + prevmousey) > maxmousespeed) mousey = prevmousey - maxmousespeed;
			if ((mousex - prevmousex) > maxmousespeed) mousex = prevmousex + maxmousespeed;
			if ((-mousex + prevmousex) > maxmousespeed) mousex = prevmousex - maxmousespeed;
		}
		else {
			mousex = prevmousex;
			mousey = prevmousey;
		}
	}
}

uint32_t get_col(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
	return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_PAINT:
	{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
		GetWindowPlacement(hWnd, &wpc);
		SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);
		SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
		if (get_key('R').held) ShowWindow(hWnd, SW_MINIMIZE);
		else ShowWindow(hWnd, SW_SHOWMAXIMIZED);

		static uint32_t old_w, old_h;
		if (gr_win.get_w() != old_w || gr_win.get_h() != old_h) {
			glViewport(0, 0, gr_win.get_w(), gr_win.get_h());
			ubo_buf.aspect = ((float)(gr_win.get_w())) / ((float)(gr_win.get_h()));
			ubo_buf.h = gr_win.get_h();
			old_w = gr_win.get_w();
			old_h = gr_win.get_h();
		}

		//>>set dt 
		static std::chrono::system_clock::time_point tp1 = std::chrono::system_clock::now();
		static std::chrono::system_clock::time_point tp2 = tp1;
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> dTime = tp2 - tp1;
		tp1 = tp2;
		float dt = dTime.count();
		//<<set dt 

		//>>key loop
		key_loop(gr_win.get_hwnd());
		//<<key loop

		if (get_key(VK_ESCAPE).held) {
			PostQuitMessage(0);
			is_alive = false;
		}
		
		ubo_buf.ang[1] += (prevmousex - mousex) * mousescale * dt;
		//ubo_buf.ang[0] += (prevmousey - mousey) * mousescale * dt;

		float a0 = prevmousey * mousescale * dt, a1 = mousey * mousescale * dt;

		ubo_buf.ang[0] += ((ubo_buf.ang[0] + a0 < a1 + 1.57079632f) && (ubo_buf.ang[0] + a0 > a1 - 1.57079632f))*(a0 - a1);
		//ubo_buf.ang[0] 
		ubo_buf.ang[1] += 6.2832853f * ((ubo_buf.ang[1] < 3.14159265f) - (ubo_buf.ang[1] > -3.14159265f));

		static float vel[3];

		vel[0] += get_key('D').held - get_key('A').held;
		vel[1] += get_key(VK_SPACE).held - get_key(VK_SHIFT).held;
		vel[2] += get_key('W').held - get_key('S').held;

		ubo_buf.pos[0] += dt * (vel[0] * cos(ubo_buf.ang[1]) - vel[2] * sin(ubo_buf.ang[1])) * speedscale;
		ubo_buf.pos[1] += dt * vel[1] * speedscale;
		ubo_buf.pos[2] += dt * (vel[2] * cos(ubo_buf.ang[1]) + vel[0] * sin(ubo_buf.ang[1])) * speedscale;

		if ((Wobj::my_unique_id != Vobj::decode_id(c.vox_stream)) && (0 != Vobj::decode_id(c.vox_stream)) && c.is_recieved) {
			std::rotate(V, V + Vobj::max_obj_amount - 1, V + Vobj::max_obj_amount);
			V[0].bytes_to_vox(c.vox_stream, &W);
			c.is_recieved = 0;
		}

		if (get_key('E').pressed || get_key('Q').held) {
			std::rotate(V, V + Vobj::max_obj_amount - 1, V + Vobj::max_obj_amount);
			V[0].x = cos(ubo_buf.ang[1] + 1.57f) * cos(ubo_buf.ang[0]) * camscale + ubo_buf.pos[0];
			V[0].y = sin(ubo_buf.ang[1] + 1.57f) * cos(ubo_buf.ang[0]) * camscale + ubo_buf.pos[2];
			V[0].z = sin(ubo_buf.ang[0]) * camscale + ubo_buf.pos[1];
			V[0].visible = 1;
			
			static char* _line;
			_line = V[0].vox_to_bytes();
			
			Bytes.body_length(Vobj::bytes_size);
			memcpy(Bytes.body(), _line, Bytes.body_length());
			Bytes.encode_header();
			c.write(Bytes);
		}

		vel[0] -= friction*vel[0];
		vel[1] -= friction*vel[1];
		vel[2] -= friction*vel[2];

		glBindBuffer(GL_UNIFORM_BUFFER, cam_ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ubo_buf), &ubo_buf);
		
		glBindProgramPipeline(shader.id);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.05, 0.05, 0.05, 1);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
		for (uint32_t ii = 0; ii < Vobj::max_obj_amount; ii++) {
			
			if (V[ii].visible) {
				ubo_buf.mod_pos[2] = V[ii].y;
				ubo_buf.mod_pos[1] = V[ii].z;
				ubo_buf.mod_pos[0] = V[ii].x;
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ubo_buf), &ubo_buf);
				//for(uint32_t i(512);i--;)
				var_o_vogl_bind_vox_buf_data(V[ii].v);
				var_o_vogl_render_var();
			}
		}

		SwapBuffers(hdc);

		InvalidateRect(hWnd, NULL, TRUE);
		std::wstring s;
		s += L", fps: ";
		s += std::to_wstring(1.0f / dt);
		TextOut(gr_win.get_hdc(), 0, 0, s.c_str(), s.length());

///////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		is_alive = false;
		break;
	case WM_SIZE:
		gr_win.resize();
		break;
	case WM_MOUSEMOVE:
		if (!get_key(VK_MENU).held) {
			if (GetCursor() != NULL) SetCursor(NULL);
		}
		else SetCursor(LoadCursor(NULL, IDC_ARROW));
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int nCmdShow) {
	gr_win.set_w(wwidth);
	gr_win.set_h(wheight);
	gr_win.init(create_wc(hInst, L"LALA", WndProc), L"LALA");

	co_win.set_w(256);
	co_win.set_h(256);
	co_win.init(create_wc(hInst, L"IHATEYOURNIGERS", WndProc), L"IHATEYOURKIM");

	gr_win.show(0);
	co_win.show(0);

	cinf.cbSize = sizeof(CURSORINFO);
///////////////////////////////////////////////////////////////////////////////////////////////////////////

	PIXELFORMATDESCRIPTOR pfd;

	hdc = gr_win.get_hdc();

	ZeroMemory(&pfd, sizeof(pfd));

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);
	hRC = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hRC);

	gladLoadGL();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	float ll = 256;

	ubo_buf.mod_mat(0, 0) = 1.0f/ll; ubo_buf.mod_mat(0, 1) = 0;       ubo_buf.mod_mat(0, 2) = 0;
	ubo_buf.mod_mat(1, 0) = 0;       ubo_buf.mod_mat(1, 1) = 1.0f/ll; ubo_buf.mod_mat(1, 2) = 0;
	ubo_buf.mod_mat(2, 0) = 0;       ubo_buf.mod_mat(2, 1) = 0;       ubo_buf.mod_mat(2, 2) = 1.0f/ll;

	ubo_buf.n = 0.1f; 
	ubo_buf.aspect = ((float)(gr_win.get_w())) / ((float)(gr_win.get_h())); 
	ubo_buf.h = gr_win.get_h();



	ubo_buf.pos[2] = -1;
	ubo_buf.ang[1] = -1;

	shader.gen(2);
	shader.create(GL_VERTEX_SHADER, GL_VERTEX_SHADER_BIT, "Shaders/main.vert.glsl", 0);
	shader.create(GL_FRAGMENT_SHADER, GL_FRAGMENT_SHADER_BIT, "Shaders/main.frag.glsl", 1);
	shader.bind();

	glGenBuffers(1, &cam_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, cam_ubo);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(ubo_buf), &ubo_buf, GL_STREAM_DRAW);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ubo_buf), &ubo_buf, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, cam_ubo);
/////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::thread webthr([/*&io_context*/]() { io_context.run(); });
	std::thread keysthr(keysfunc);


/////////////////////////////////////////////////////////////////////////////////////////////////////////
	for (uint32_t ii = 0; ii < Vobj::max_obj_amount; ii++) {
		V[ii].fill_vox_data();
		v[ii].init(V[ii].w, V[ii].h, V[ii].d);
		v[ii].zero();
	
		float inv_a = 1.0f / ll;
		float inv_b = 1.0f / 16;
	
		for (uint32_t i(v[ii].w); i--;)
			for (uint32_t j(v[ii].h); j--;)
				for (uint32_t k(v[ii].d); k--;)
					if (nois.noise(i * inv_a, k * inv_a) + 1 > 2 * j * inv_a)
						v[ii](i, j, k) = get_col(0, 0xFF * 0.333f * (nois.noise(i * inv_b, j * inv_b, k * inv_b) + 2), ii * 0x36, 0xFF);
	
		var_o_vogl_init_var_renderer();
		var_o_vogl_bind_vox_buf_data(&v[ii]);
		V[ii].bind_vox_data(&v[ii]);
	}
	//Cam_vox.fill_vox_data();
	//cam_vox.init(Cam_vox.w, Cam_vox.h, Cam_vox.d);
	//cam_vox.zero();
	//for (uint32_t i(cam_vox.w); i--;)
	//	for (uint32_t j(cam_vox.h); j--;)
	//		for (uint32_t k(cam_vox.d); k--;)
	//			cam_vox(i, j, k) = get_col(0xFF, 0, 0, 0xFF);
	//var_o_vogl_init_var_renderer();
	//var_o_vogl_bind_vox_buf_data(&cam_vox);
	//Cam_vox.bind_vox_data(&cam_vox);

	//for (uint32_t ii = 0; ii < Vobj::max_obj_amount; ii++) {
	//	V[ii].fill_vox_data();
	//	v[0].init(V[ii].w, V[ii].h, V[ii].d);
	//	v[0].zero();
	//
	//	float inv_a = 1.0f / ll;
	//	float inv_b = 1.0f / 16;
	//
	//	for (uint32_t i(v[0].w); i--;)
	//		for (uint32_t j(v[0].h); j--;)
	//			for (uint32_t k(v[0].d); k--;)
	//				if (nois.noise(i * inv_a, k * inv_a) + 1 > 2 * j * inv_a)
	//					v[0](i, j, k) = get_col(0, 0xFF * 0.333f * (nois.noise(i * inv_b, j * inv_b, k * inv_b) + 2), ii * 0x36, 0xFF);
	//
	//	V[ii].bind_vox_data(&v[0]);
	//}
	//var_o_vogl_init_var_renderer();
	//var_o_vogl_bind_vox_buf_data(&v[0]);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
	MSG msg;
	do {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		RedrawWindow(gr_win.get_hwnd(), nullptr, nullptr, RDW_INTERNALPAINT);
	} while (msg.message != WM_QUIT);

///////////////////////////////////////////////////////////////////////////////////////////////////////////

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(gr_win.get_hwnd(), GetDC(gr_win.get_hwnd()));
	DestroyWindow(gr_win.get_hwnd());

///////////////////////////////////////////////////////////////////////////////////////////////////////////
	c.close();
	webthr.join();
	keysthr.join();
	return msg.wParam;
}