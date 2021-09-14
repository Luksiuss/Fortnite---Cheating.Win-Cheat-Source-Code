#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <tchar.h>
#include <winioctl.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <dwmapi.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dwmapi.lib")

#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_dx9.h"
#include "Imgui/imgui_impl_win32.h"

#include "font.h"
#include "comm.hpp"
#include "utils.hpp"

IDirect3D9Ex* p_Object = NULL;
IDirect3DDevice9Ex* p_Device = NULL;
D3DPRESENT_PARAMETERS p_Params = { NULL };

HWND MyWnd = NULL;
HWND GameWnd = NULL;
RECT GameRect = { NULL };
MSG Message = { NULL };
const MARGINS Margin = { -1 };

// link external functions
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// prototypes
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
void CleanD3D();

HRESULT DirectXInit(HWND hWnd) {
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&p_Params, sizeof(p_Params));
	p_Params.Windowed = TRUE;
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_Params.hDeviceWindow = hWnd;
	p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_Params.BackBufferWidth = Settings::MajorValues::Width;
	p_Params.BackBufferHeight = Settings::MajorValues::Height;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;
	p_Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
	{
		p_Object->Release();
		exit(4);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImFontConfig font_config;
	font_config.OversampleH = 1;
	font_config.OversampleV = 1;
	font_config.PixelSnapH = true;

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF,
		0x0400, 0x044F,
		0,
	};

	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 13.f);

	Settings::MajorValues::SkeetFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(icon_compressed_data, icon_compressed_size, 15.f, &font_config, ranges);

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(p_Device);

	ImGui::StyleColorsClassic();
	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 0.0f;
	style->FramePadding = ImVec2(2, 2);
	style->FrameRounding = 0.0f;
	style->ItemSpacing = ImVec2(8, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 0.0f;
	style->GrabMinSize = 10.0f;
	style->GrabRounding = 0.0f;
	style->TabRounding = 0.f;
	style->ChildRounding = 0.f;

	style->Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImColor(40, 40, 40, 255);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImColor(150, 0, 0, 255);
	style->Colors[ImGuiCol_SliderGrab] = ImColor(40, 40, 40, 255);
	style->Colors[ImGuiCol_SliderGrabActive] = ImColor(60, 60, 60, 255);
	style->Colors[ImGuiCol_Button] = ImColor(40, 40, 40, 255);
	style->Colors[ImGuiCol_ButtonHovered] = ImColor(50, 50, 50, 255);
	style->Colors[ImGuiCol_ButtonActive] = ImColor(50, 50, 50, 255);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

	style->WindowTitleAlign.x = 0.50f;
	style->FrameRounding = 2.0f;

	p_Object->Release();
	return S_OK;
}

void SetupWindow()
{
WNDCLASSEX wClass =
	{
		sizeof(WNDCLASSEX),
		0,
		WinProc,
		0,
		0,
		nullptr,
		LoadIcon(nullptr, IDI_APPLICATION),
		LoadCursor(nullptr, IDC_ARROW),
		nullptr,
		nullptr,
		TEXT("Test1"),
		LoadIcon(nullptr, IDI_APPLICATION)
	};

	if (!RegisterClassEx(&wClass))
		exit(1);

	GameWnd = FindWindowW(NULL, TEXT("Fortnite  "));

	//printf("GameWnd Found! : %p\n", GameWnd);

	if (GameWnd)
	{
		GetClientRect(GameWnd, &GameRect);
		POINT xy;
		ClientToScreen(GameWnd, &xy);
		GameRect.left = xy.x;
		GameRect.top = xy.y;

		Settings::MajorValues::Width = GameRect.right;
		Settings::MajorValues::Height = GameRect.bottom;
	}
	else exit(2);

	MyWnd = CreateWindowExA(NULL, "Test1", "Test1", WS_POPUP | WS_VISIBLE, GameRect.left, GameRect.top, Settings::MajorValues::Width, Settings::MajorValues::Height, NULL, NULL, 0, NULL);
	DwmExtendFrameIntoClientArea(MyWnd, &Margin);
	SetWindowLong(MyWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	ShowWindow(MyWnd, SW_SHOW);
	UpdateWindow(MyWnd);

	//printf("Hwnd created : %p\n", MyWnd);
}

void actorLoop() {
	std::vector<ActorStruct> actorStructVector;

	uintptr_t uWorld = read<uintptr_t>(m_base + 0x93538b0);
	if (!uWorld) {
		return;
	}

	//printf("uWorld :0x%llX\n", uWorld);

	uintptr_t PersistentLevel = read<uintptr_t>(uWorld + 0x30);
	if (!PersistentLevel) {
		return;
	}

	//printf("PersistentLevel :0x%llX\n", PersistentLevel);

	uintptr_t OwningGameInstance = read<uintptr_t>(uWorld + 0x180);
	if (!OwningGameInstance) {
		return;
	}

	uintptr_t LocalPlayers = read<uintptr_t>(OwningGameInstance + 0x38);
	if (!LocalPlayers) {
		return;
	}

	Settings::MajorValues::LocalPlayer = read<uintptr_t>(LocalPlayers);
	if (!Settings::MajorValues::LocalPlayer) {
		return;
	}

	uintptr_t LocalPlayerController = read<uintptr_t>(Settings::MajorValues::LocalPlayer + 0x30);
	if (!LocalPlayerController) {
		return;
	}

	Settings::MajorValues::LocalPawn = read<uintptr_t>(LocalPlayerController + 0x2A0);
	if (!Settings::MajorValues::LocalPawn) {
		return;
	}
	else {
		Settings::MajorValues::LocalPawnRootComponent = read<uintptr_t>(Settings::MajorValues::LocalPawn + 0x130);
		Settings::MajorValues::LocalPlayerRelativeLocation = read<Vector3>(Settings::MajorValues::LocalPawnRootComponent + 0x11C);

		Settings::MajorValues::LocalPlayerID = read<int>(Settings::MajorValues::LocalPawn + 0x18);
	}

	uint64_t localplayerstate = read<uint64_t>(Settings::MajorValues::LocalPawn + 0x240);
	int LocalTeam = read<int>(localplayerstate + 0xED0);

	for (int index = 0; index < read<int>(PersistentLevel + (0x98 + sizeof(uintptr_t))); index++)
	{
		uintptr_t PersistentLevelActors = read<uintptr_t>(PersistentLevel + 0x98);
		if (!PersistentLevelActors) {
			return;
		}

		uintptr_t CurrentActor = read<uintptr_t>(PersistentLevelActors + (index * sizeof(uintptr_t)));
		if (!CurrentActor) {
			continue;
		}

		uintptr_t CurrentActorMesh = read<uintptr_t>(CurrentActor + 0x280);
		if (!CurrentActorMesh) {
			continue;
		}

		int CurrentActorID = read<int>(CurrentActor + 0x18);
		if (!CurrentActorID) {
			continue;
		}

		bool bSpotted = read<bool>(CurrentActor + 0x53a); // FortPawn:bSpotted
		if (!bSpotted) {
			continue;
		}

		if (CurrentActorID != 0 && ((CurrentActorID == Settings::MajorValues::LocalPlayerID) || (bSpotted != 0 && Settings::MajorValues::CorrectbSpotted == bSpotted))) {
			Settings::MajorValues::CorrectbSpotted = bSpotted;

			ActorStruct Actor{ };
			Actor.pObjPointer = CurrentActor;
			Actor.ID = CurrentActorID;
			Actor.Mesh = CurrentActorMesh;

			actorStructVector.push_back(Actor);
		}
	}

	if (actorStructVector.empty()) {
		return;
	}

	bool bValidEnemyInArea = false;
	float ClosestActorDistance = FLT_MAX;
	Vector3 ClosestActorMouseAimbotPosition = Vector3(0.0f, 0.0f, 0.0f);
	float distance;

	for (const ActorStruct& ActorStruct : actorStructVector)
	{
		if (ActorStruct.pObjPointer == Settings::MajorValues::LocalPawn) {
			continue;
		}

		uintptr_t RootComponent = read<uintptr_t>(ActorStruct.pObjPointer + 0x130);
		if (!RootComponent) {
			continue;
		}

		uint64_t playerstate = read<uint64_t>(ActorStruct.pObjPointer + 0x240);
		int TeamIndex = read<int>(playerstate + 0xED0);

		Vector3 vHeadBone = GetBoneWithRotation(ActorStruct.Mesh, 96);
		Vector3 vRootBone = GetBoneWithRotation(ActorStruct.Mesh, 0);

		Vector3 vHeadBoneOut = ProjectWorldToScreen(Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z));
		Vector3 vRootBoneOut = ProjectWorldToScreen(vRootBone);

		Vector3 RootPos = GetBoneWithRotation(ActorStruct.Mesh, select_hitbox());
		Vector3 selection;
		
		float BoxHeight = abs(vHeadBoneOut.y - vRootBoneOut.y);
		float BoxWidth = BoxHeight / 1.8f;

		Vector3 RelativeInternalLocation = read<Vector3>(RootComponent + 0x11C);
		if (!RelativeInternalLocation.x && !RelativeInternalLocation.y) {
			continue;
		}

		Vector3 RelativeScreenLocation = ProjectWorldToScreen(RelativeInternalLocation);
		if (!RelativeScreenLocation.x && !RelativeScreenLocation.y) {
			continue;
		}

		distance = Settings::MajorValues::LocalPlayerRelativeLocation.Distance(RelativeInternalLocation) / 100.f;

		if (TeamIndex != LocalTeam) {

			if (distance <= Settings::Selection::EspDistance) {

				if (Settings::Selection::bLineESPEnabled) {
					ImGui::GetOverlayDrawList()->AddLine(
						ImVec2(Settings::MajorValues::ScreenCenterX, Settings::MajorValues::Height),
						ImVec2(RelativeScreenLocation.x, RelativeScreenLocation.y),
						ImGui::ColorConvertFloat4ToU32(ImVec4(ESPColor.R / 255.0, ESPColor.G / 255.0, ESPColor.B / 255.0, ESPColor.A / 255.0)),
						1
					);
				}

				if (Settings::Selection::Prediction) {

					Vector3 Velocity = read<Vector3>(ActorStruct.pObjPointer + 0x140);
					auto Result = CalculatePrediction(RootPos, Velocity, distance, 275.0f);
					selection = ProjectWorldToScreen(Result);
				}

				float ScreenLocationX = selection.x - Settings::MajorValues::ScreenCenterX, ScreenLocationY = selection.y - Settings::MajorValues::ScreenCenterY;
				float ActorDistance = std::sqrtf(ScreenLocationX * ScreenLocationX + ScreenLocationY * ScreenLocationY);

				if (Settings::Selection::bVisualName)
				{
					char name[64];
					sprintf_s(name, "Player - %2.fm", distance);
					DrawString(14, vHeadBoneOut.x, vHeadBoneOut.y - 15, &Col.white_, true, true, name);
				}

				if (Settings::Selection::bBox)
				{
					if (Settings::Selection::bBoxMode == 0 || Settings::Selection::bBoxMode == 1)
					{
						if (Settings::Selection::outline)
						{
							DrawNormalBox(vRootBoneOut.x - BoxWidth / 2 + 1, vHeadBoneOut.y, BoxWidth, BoxHeight, 1.0f, &Col.black);
							DrawNormalBox(vRootBoneOut.x - BoxWidth / 2 - 1, vHeadBoneOut.y, BoxWidth, BoxHeight, 1.0f, &Col.black);
							DrawNormalBox(vRootBoneOut.x - BoxWidth / 2, vHeadBoneOut.y + 1, BoxWidth, BoxHeight, 1.0f, &Col.black);
							DrawNormalBox(vRootBoneOut.x - BoxWidth / 2, vHeadBoneOut.y - 1, BoxWidth, BoxHeight, 1.0f, &Col.black);
						}
						DrawNormalBox(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, 1.0f, &ESPColor);
					}
					if (Settings::Selection::bBoxMode == 2 || Settings::Selection::bBoxMode == 3)
					{
						if (Settings::Selection::outline)
						{
							DrawCornerBox(vRootBoneOut.x - BoxWidth / 2 + 1, vHeadBoneOut.y, BoxWidth, BoxHeight, 1.0f, &Col.black);
							DrawCornerBox(vRootBoneOut.x - BoxWidth / 2 - 1, vHeadBoneOut.y, BoxWidth, BoxHeight, 1.0f, &Col.black);
							DrawCornerBox(vRootBoneOut.x - BoxWidth / 2, vHeadBoneOut.y + 1, BoxWidth, BoxHeight, 1.0f, &Col.black);
							DrawCornerBox(vRootBoneOut.x - BoxWidth / 2, vHeadBoneOut.y - 1, BoxWidth, BoxHeight, 1.0f, &Col.black);
						}
						DrawCornerBox(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, 1.0f, &ESPColor);
					}
				}

				if (Settings::Selection::showhead)
					DrawCircle(vHeadBoneOut.x, vHeadBoneOut.y, BoxHeight / 25, &Col.white_, 15);

				if (Settings::Selection::skel) {

					if (distance <= Settings::Selection::MaxSkeletonDrawDistance) {

						Vector3 vHeadBone = GetBoneWithRotation(ActorStruct.Mesh, 96);
						Vector3 vHeadBoneOut = ProjectWorldToScreen(vHeadBone);

						Vector3 vHip = GetBoneWithRotation(ActorStruct.Mesh, 2);
						Vector3 vHipOut = ProjectWorldToScreen(vHip);

						Vector3 vNeck = GetBoneWithRotation(ActorStruct.Mesh, 65);
						Vector3 vNeckOut = ProjectWorldToScreen(vNeck);

						Vector3 vUpperArmLeft = GetBoneWithRotation(ActorStruct.Mesh, 34);
						Vector3 vUpperArmLeftOut = ProjectWorldToScreen(vUpperArmLeft);

						Vector3 vUpperArmRight = GetBoneWithRotation(ActorStruct.Mesh, 91);
						Vector3 vUpperArmRightOut = ProjectWorldToScreen(vUpperArmRight);

						Vector3 vLeftHand = GetBoneWithRotation(ActorStruct.Mesh, 35);
						Vector3 vLeftHandOut = ProjectWorldToScreen(vLeftHand);

						Vector3 vRightHand = GetBoneWithRotation(ActorStruct.Mesh, 63);
						Vector3 vRightHandOut = ProjectWorldToScreen(vRightHand);

						Vector3 vLeftHand1 = GetBoneWithRotation(ActorStruct.Mesh, 33);
						Vector3 vLeftHandOut1 = ProjectWorldToScreen(vLeftHand1);

						Vector3 vRightHand1 = GetBoneWithRotation(ActorStruct.Mesh, 60);
						Vector3 vRightHandOut1 = ProjectWorldToScreen(vRightHand1);

						Vector3 vRightThigh = GetBoneWithRotation(ActorStruct.Mesh, 74);
						Vector3 vRightThighOut = ProjectWorldToScreen(vRightThigh);

						Vector3 vLeftThigh = GetBoneWithRotation(ActorStruct.Mesh, 67);
						Vector3 vLeftThighOut = ProjectWorldToScreen(vLeftThigh);

						Vector3 vRightCalf = GetBoneWithRotation(ActorStruct.Mesh, 75);
						Vector3 vRightCalfOut = ProjectWorldToScreen(vRightCalf);

						Vector3 vLeftCalf = GetBoneWithRotation(ActorStruct.Mesh, 68);
						Vector3 vLeftCalfOut = ProjectWorldToScreen(vLeftCalf);

						Vector3 vLeftFoot = GetBoneWithRotation(ActorStruct.Mesh, 69);
						Vector3 vLeftFootOut = ProjectWorldToScreen(vLeftFoot);

						Vector3 vRightFoot = GetBoneWithRotation(ActorStruct.Mesh, 76);
						Vector3 vRightFootOut = ProjectWorldToScreen(vRightFoot);

						Vector3 Pelvis = GetBoneWithRotation(ActorStruct.Mesh, 2);
						Vector3 PelvisOut = ProjectWorldToScreen(Pelvis);

						DrawLine(vNeckOut.x, vNeckOut.y, vHeadBoneOut.x, vHeadBoneOut.y, &Col.white_, 2.0f);
						DrawLine(PelvisOut.x, PelvisOut.y, vNeckOut.x, vNeckOut.y, &Col.white_, 2.0f);

						DrawLine(vUpperArmLeftOut.x, vUpperArmLeftOut.y, vNeckOut.x, vNeckOut.y, &Col.white_, 2.0f);
						DrawLine(vUpperArmRightOut.x, vUpperArmRightOut.y, vNeckOut.x, vNeckOut.y, &Col.white_, 2.0f);

						DrawLine(vLeftHandOut.x, vLeftHandOut.y, vUpperArmLeftOut.x, vUpperArmLeftOut.y, &Col.white_, 2.0f);
						DrawLine(vRightHandOut.x, vRightHandOut.y, vUpperArmRightOut.x, vUpperArmRightOut.y, &Col.white_, 2.0f);

						DrawLine(vLeftHandOut.x, vLeftHandOut.y, vLeftHandOut1.x, vLeftHandOut1.y, &Col.white_, 2.0f);
						DrawLine(vRightHandOut.x, vRightHandOut.y, vRightHandOut1.x, vRightHandOut1.y, &Col.white_, 2.0f);

						DrawLine(vLeftThighOut.x, vLeftThighOut.y, vHipOut.x, vHipOut.y, &Col.white_, 2.0f);
						DrawLine(vRightThighOut.x, vRightThighOut.y, vHipOut.x, vHipOut.y, &Col.white_, 2.0f);

						DrawLine(vLeftCalfOut.x, vLeftCalfOut.y, vLeftThighOut.x, vLeftThighOut.y, &Col.white_, 2.0f);
						DrawLine(vRightCalfOut.x, vRightCalfOut.y, vRightThighOut.x, vRightThighOut.y, &Col.white_, 2.0f);

						DrawLine(vLeftFootOut.x, vLeftFootOut.y, vLeftCalfOut.x, vLeftCalfOut.y, &Col.white_, 2.0f);
						DrawLine(vRightFootOut.x, vRightFootOut.y, vRightCalfOut.x, vRightCalfOut.y, &Col.white_, 2.0f);

					}
				}

				if (Settings::Selection::bBoxMode == 1 || Settings::Selection::bBoxMode == 3)
					DrawFilledRect(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, &Col.filled);

				if (ActorDistance < ClosestActorDistance && ActorDistance < Settings::Selection::AimbotFOVValue) {
					ClosestActorDistance = ActorDistance;
					ClosestActorMouseAimbotPosition = Vector3(ScreenLocationX, ScreenLocationY, 0.0f);
					bValidEnemyInArea = true;
				}
			}
		}
	}

	if (Settings::Selection::bMouseAimbotEnabled && bValidEnemyInArea && GetAsyncKeyState(hotkeys::aimkey)) {
		float PlayerLocationX = ClosestActorMouseAimbotPosition.x /= Settings::Selection::AimbotSmoothingValue, PlayerLocationY = ClosestActorMouseAimbotPosition.y /= Settings::Selection::AimbotSmoothingValue;

		if (!PlayerLocationX || !PlayerLocationY) {
			return;
		}

		mouse_event(MOUSEEVENTF_MOVE, PlayerLocationX, PlayerLocationY, 0, 0);
	}
}

bool draw_menu() {

	static int MenuTab = 0;
	static int VisualTab = 0;
	float
		TextSpaceLine = 90.f,
		SpaceLineOne = 120.f,
		SpaceLineTwo = 280.f,
		SpaceLineThr = 420.f;
	ImGuiStyle* style = &ImGui::GetStyle();
	static auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar;

	//if (Settings::MajorValues::menuIsOpen)
	{
		if (ImGui::Begin("cheating.win")) {

			ImGui::SetWindowSize(ImVec2(590, 315), ImGuiCond_Once);
			{
				ImGui::BeginChild(("##Tabs"), ImVec2(-1, 30.0f));
				{
					style->ItemSpacing = ImVec2(1, 1);
					ImGui::PushFont(Settings::MajorValues::SkeetFont);
					{
						if (ImGui::Button((("! Aimbot")), ImVec2(185, 20)))
						{
							MenuTab = 0;
						}ImGui::SameLine();
						if (ImGui::Button((("$ Visuals")), ImVec2(185, 20)))
						{
							MenuTab = 1;
						}ImGui::SameLine();
						if (ImGui::Button((("% Misc")), ImVec2(185, 20)))
						{
							MenuTab = 2;
						}ImGui::SameLine();
					} ImGui::PopFont();

				} ImGui::EndChild();

				style->ItemSpacing = ImVec2(8, 8);

				if (MenuTab == 0)
				{
					ImGui::SetCursorPos(ImVec2(10, 50));
					ImGui::Text(("Aimbot:"));
					ImGui::SetCursorPos(ImVec2(10, 70));
					ImGui::BeginChild(("##Aimbot"), ImVec2(130.f, 200.f), true);
					{
						ImGui::Checkbox(("Aimbot"), &Settings::Selection::bMouseAimbotEnabled);
						ImGui::Checkbox(("Show FOV"), &Settings::Selection::bDrawFOV);
					}
				}
			}

			ImGui::End();
		}
	}
	return true;
}

void renderLoopCall() {
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	RECT rect = { 0 };

	if (GetWindowRect(GameWnd, &rect))
	{
		Settings::MajorValues::Width = rect.right - rect.left;
		Settings::MajorValues::Height = rect.bottom - rect.top;
	}

	Settings::MajorValues::ScreenCenterX = (Settings::MajorValues::Width / 2.0f), Settings::MajorValues::ScreenCenterY = (Settings::MajorValues::Height / 2.0f);

	//if (GetAsyncKeyState(VK_INSERT) & 1) {
	//	Settings::MajorValues::menuIsOpen = !Settings::MajorValues::menuIsOpen;
	//}

	actorLoop();

	if (Settings::Selection::bDrawFOV) {
		ImGui::GetOverlayDrawList()->AddCircle(ImVec2(Settings::MajorValues::ScreenCenterX, Settings::MajorValues::ScreenCenterY), Settings::Selection::AimbotFOVValue, ImGui::ColorConvertFloat4ToU32(ImVec4(ESPColor.R / 255.0, ESPColor.G / 255.0, ESPColor.B / 255.0, ESPColor.A / 255.0)), 100);
	}

	draw_menu();
	

	ImGui::EndFrame();

	p_Device->SetRenderState(D3DRS_ZENABLE, false);
	p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	p_Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	if (p_Device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		p_Device->EndScene();
	}

	HRESULT result = p_Device->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && p_Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		p_Device->Reset(&p_Params);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

WPARAM MainLoop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, MyWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwndActive = GetForegroundWindow();

		if (hwndActive == GameWnd) {
			HWND hwndTest = GetWindow(hwndActive, GW_HWNDPREV);

			SetWindowPos(MyWnd, hwndTest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(GameWnd, &rc);
		ClientToScreen(GameWnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameWnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState(VK_LBUTTON)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;

			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else {
			io.MouseDown[0] = false;
		}
		renderLoopCall();
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanD3D();
	DestroyWindow(MyWnd);

	return Message.wParam;
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam)) {
		return true;
	}

	switch (Message)
	{
	case WM_DESTROY:
		CleanD3D();
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (p_Device != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_Params.BackBufferWidth = LOWORD(lParam);
			p_Params.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = p_Device->Reset(&p_Params);

			if (hr == D3DERR_INVALIDCALL) {
				IM_ASSERT(0);
			}

			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}

	return 0;
}

void CleanD3D() {
	if (p_Device != NULL)
	{
		p_Device->EndScene();
		p_Device->Release();
	}

	if (p_Object != NULL)
	{
		p_Object->Release();
	}
}

bool initiateCheat() {

	m_driver_control = kernel_control_function();

	if (!kernel_control_function()) {
		std::cout << "[-] Failed to initiate communication with driver." << std::endl;

		system("pause");

		return false;
	}

	std::cout << "[=] Looking for Fortnite." << std::endl;

	while (!m_pid) {
		m_pid = GetAowProcId();
	}

	std::cout << "[+] Fortnite PID : " << m_pid << std::endl;

	usermode_pid = GetCurrentProcessId();

	std::cout << "[+] UserMode PID : " << usermode_pid << std::endl;

	std::cout << "[=] Getting Fortnite base address." << std::endl;

	while (!m_base) {
		m_base = GetBaseAddress();
	}

	std::cout << "[+] Fortnite base address : " << m_base << std::endl;

	return true;
}

int main() {
	std::cout << "[+] At program entry!" << std::endl;

	if (!initiateCheat()) {
		std::cout << "[-] Cheat initiation failled!" << std::endl;

		system("pause");

		return 0;
	}

	SetupWindow();
	DirectXInit(MyWnd);

	while (true) {
		MainLoop();
	}

	exit(0);

	return 0;
}