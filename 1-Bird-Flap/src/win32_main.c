#include <windows.h>

#define UNICODE
#define _UNICODE

#define internal static

typedef char		s8;
typedef short		s16;
typedef long		s32;
typedef __int64		s64;

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned long		u32;
typedef unsigned __int64	u64;

typedef float	f32;
typedef double	f64;

typedef struct GameRenderer {
	int width;
	int height;
	void *data;
	BITMAPINFO bitmap_info;
} GameRenderer;

typedef struct GameState {
	BOOL run;
	u8 kd_down;
	u8 kd_up;
	u8 kd_right;
	u8 kd_left;
} GameState;

typedef struct Player {
	int posx;
	int posy;
	int size;
} Player;

static GameRenderer renderer;
static GameState game_state;
static Player player;

void
resize_render_dib_section(HDC device_context, GameRenderer *renderer, int width, int height) {
	int alloc_size;

	if (renderer->data) {
		if (FAILED(VirtualFree(renderer->data, 0, MEM_RELEASE))) {
			OutputDebugStringA("Failed to free renderer dib section allocation.\n");
		}
		renderer->data = NULL;
	}

	alloc_size = width * height * 4;
	renderer->data = VirtualAlloc(NULL, alloc_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (!renderer) {
		OutputDebugStringA("Failed to allocate renderer dib section.\n");
	}

	renderer->bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	renderer->bitmap_info.bmiHeader.biWidth = width;
	renderer->bitmap_info.bmiHeader.biHeight = -height;
	renderer->bitmap_info.bmiHeader.biPlanes = 1;
	renderer->bitmap_info.bmiHeader.biBitCount = 32;
	renderer->bitmap_info.bmiHeader.biCompression = BI_RGB;
	renderer->bitmap_info.bmiHeader.biSizeImage = 0;

	renderer->width = width;
	renderer->height = height;

	// HBITMAP ds = CreateDIBSection(
	// 	device_context,
	// 	&renderer->bitmap_info,
	// 	DIB_RGB_COLORS,
	// 	renderer->data,
	// 	NULL,
	// 	0
	// );
}

void
fill_renderer_back(GameRenderer *renderer, int width, int height) {
	int i;
	int j;
	int pitch;
	u8 *row;
	u8 *pixel;

	row = (u8 *)renderer->data;
	pitch = width * 4;

	for (i = 0; i < height; ++i) {
		pixel = (u8 *)row;
		for (j = 0; j < width; ++j) {
			if (
				(j >= player.posx && j <= player.posx + player.size)
				&& (i >= player.posy && i <= player.posy + player.size)
			) {
				*pixel = 0;
				++pixel;
				*pixel = 255;
				++pixel;
				*pixel = 0;
				++pixel;
				*pixel = 255;
				++pixel;
			} else {
				*pixel = 255;
				++pixel;
				*pixel = 255;
				++pixel;
				*pixel = 255;
				++pixel;
				*pixel = 255;
				++pixel;
			}
		}

		row += pitch;
	}
}

void
renderer_to_display(HDC device_context, GameRenderer *renderer, int width, int height) {
	StretchDIBits(
		device_context,
		0, 0,
		width, height,
		0, 0,
		renderer->width, renderer->height,
		renderer->data,
		&renderer->bitmap_info,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

LRESULT CALLBACK
window_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) {
	switch (message) {
	case WM_DESTROY:
		game_state.run = FALSE;
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		game_state.run = FALSE;
		PostQuitMessage(0);
		return 0;

	case WM_SIZE: {
		HDC device_context;
		RECT client_rect;
		int width;
		int height;

		device_context = GetDC(hwnd);
		GetClientRect(hwnd, &client_rect);

		width = client_rect.right - client_rect.left;
		height = client_rect.bottom - client_rect.top;

		resize_render_dib_section(device_context, &renderer, width, height);

		ReleaseDC(hwnd, device_context);
		return 0;
	}

	case WM_PAINT:
		HDC device_context;
		RECT client_rect;
		PAINTSTRUCT ps;
		int width, height;

		device_context = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &client_rect);

		width = client_rect.right - client_rect.left;
		height = client_rect.bottom - client_rect.top;

		renderer_to_display(device_context, &renderer, width, height);
		EndPaint(hwnd, &ps);

		return 0;

	case WM_KEYDOWN:
		if (w_param == VK_RIGHT)
			// player.posx += 5;
			game_state.kd_right = 1;
		if (w_param == VK_LEFT)
			// player.posx -= 5;
			game_state.kd_left = 1;
		if (w_param == VK_UP)
			// player.posy -= 5;
			game_state.kd_up = 1;
		if (w_param == VK_DOWN)
			// player.posy += 5;
			game_state.kd_down = 1;
		return 0;
	case WM_KEYUP:
		if (w_param == VK_RIGHT)
			// player.posx += 5;
			game_state.kd_right = 0;
		if (w_param == VK_LEFT)
			// player.posx -= 5;
			game_state.kd_left = 0;
		if (w_param == VK_UP)
			// player.posy -= 5;
			game_state.kd_up = 0;
		if (w_param == VK_DOWN)
			// player.posy += 5;
			game_state.kd_down = 0;
		return 0;

	default:
		return DefWindowProcW(hwnd, message, w_param, l_param);
	}

	return 0;
}

int WINAPI
wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR command_line, int command_show) {
	WNDCLASSEXW win_class;
	HWND hwnd;
	const wchar_t *class_name = L"BirdFlap";
	const wchar_t *window_name = L"Bird Flap";

	if (prev_instance) {
		OutputDebugStringA("This program cannot be run in DOS.\n");
	}

	win_class.cbSize = sizeof(WNDCLASSEXW);
	win_class.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	win_class.lpfnWndProc = window_proc;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = instance;
	win_class.hIcon = NULL;
	win_class.hCursor = NULL;
	win_class.hbrBackground = NULL;
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = class_name;
	win_class.hIconSm = NULL;

	if (FAILED(RegisterClassExW(&win_class))) {
		OutputDebugStringA("Failed to Win32 register class.\n");
	}

	hwnd = CreateWindowW(
		class_name,
		window_name,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 800,
		NULL,
		NULL,
		instance,
		NULL
	);

	if (!hwnd) {
		OutputDebugStringA("Failed to create window.\n");
	}

	ShowWindow(hwnd, command_show);

	player.posx = 64;
	player.posy = 64;
	player.size = 32;

	game_state.kd_down = 0;
	game_state.kd_up = 0;
	game_state.kd_right = 0;
	game_state.kd_left = 0;

	MSG msg;
	HDC device_context;
	device_context = GetDC(hwnd);
	game_state.run = TRUE;
	while (game_state.run) {
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		RECT client_rect;
		PAINTSTRUCT ps;
		int width;
		int height;

		if (game_state.kd_down == 1)
			player.posy += 5;
		if (game_state.kd_up == 1)
			player.posy -= 5;
		if (game_state.kd_right == 1)
			player.posx += 5;
		if (game_state.kd_left == 1)
			player.posx -= 5;

		device_context = GetDC(hwnd);
		GetClientRect(hwnd, &client_rect);

		width = client_rect.right - client_rect.left;
		height = client_rect.bottom - client_rect.top;

		fill_renderer_back(&renderer, width, height);
		renderer_to_display(device_context, &renderer, width, height);
	}

	return 0;
}
