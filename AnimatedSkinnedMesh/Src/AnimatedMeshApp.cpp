#include "AnimatedMeshApp.h"
#include "UnknownInstance\Pos.h"
#include "SkinnedEffect.h"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	AnimatedMeshApp myApp(hInstance);

	if (!myApp.Init())
	{
		return 0;
	}

	return myApp.Run();
}

AnimatedMeshApp::AnimatedMeshApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
	, mRotateCameraController(*this, 1, 200)
	, mCharacterModel(nullptr)
{
	mCamera.SetPosition(0.0f, 2.0f, -15.0f);
	SetCameraControll(&mRotateCameraController);

	mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f);

	mEnvironment.dirLights[0].Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mEnvironment.dirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.4f, 1.0f);
	mEnvironment.dirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	mEnvironment.dirLights[0].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mEnvironment.dirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mEnvironment.dirLights[1].Diffuse = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
	mEnvironment.dirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mEnvironment.dirLights[1].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mEnvironment.dirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mEnvironment.dirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mEnvironment.dirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mEnvironment.dirLights[2].Direction = XMFLOAT3(0.0f, 0.0, -1.0f);
}

AnimatedMeshApp::~AnimatedMeshApp()
{
	SafeDelete(mCharacterModel);
}

bool AnimatedMeshApp::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	mTextureMgr.Init(md3dDevice);

	mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	AddUnknownInstance(new SkyEffect(md3dDevice, L"FX/Sky.fx"));
	AddUnknownInstance(new SkinnedEffect(md3dDevice, L"FX/SkinnedEffect.fx"));

	mEnvironment.Apply(SkinnedEffect::Ptr());

	std::vector<std::wstring> fileNames;
	fileNames.push_back(L"Textures/desertcube1024.dds");
	mSky.Build(this, fileNames, 5000.0f);

	mCharacterModel = new SkinnedModel(md3dDevice, mTextureMgr, "Models\\soldier.m3d", L"Textures\\");
	mCharacterInstance.Model = mCharacterModel;
	mCharacterInstance.TimePos = 0.0f;
	mCharacterInstance.ClipName = "Take1";
	mCharacterInstance.FinalTransforms.resize(mCharacterModel->SkinnedData.BoneCount());

	// Reflect to change coordinate system from the RHS the data was exported out as.
	XMMATRIX modelScale = XMMatrixScaling(0.05f, 0.05f, -0.05f);
	XMMATRIX modelRot = XMMatrixRotationY(MathHelper::Pi);
	XMMATRIX modelOffset = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
	XMStoreFloat4x4(&mCharacterInstance.World, modelScale * modelRot * modelOffset);

	return true;
}

void AnimatedMeshApp::DrawScene()
{
	//Draw normal scenes
	ID3D11RenderTargetView* renderTargets[1] = { mRenderTargetView };
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//SKY
	mSky.Draw(this);

	//
	// Draw the animated characters.
	//
	md3dImmediateContext->IASetInputLayout(PosNormalTexTanSkinned::GetInputLayout());

	ID3DX11EffectTechnique* animatedTech = SkinnedEffect::Ptr()->Light1TexSkinnedTech;

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX view = mCamera.View();
	XMMATRIX proj = mCamera.Proj();
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldView;
	XMMATRIX worldInvTransposeView;
	XMMATRIX worldViewProj;

	D3DX11_TECHNIQUE_DESC techDesc;
	animatedTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Instance 1

		world = XMLoadFloat4x4(&mCharacterInstance.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView = world * view;
		worldInvTransposeView = worldInvTranspose * view;
		worldViewProj = world * view * proj;

		SkinnedEffect::Ptr()->SetWorld(world);
		SkinnedEffect::Ptr()->SetWorldInvTranspose(worldInvTranspose);
		SkinnedEffect::Ptr()->SetWorldViewProj(worldViewProj);
		SkinnedEffect::Ptr()->SetWorldViewProjTex(worldViewProj * toTexSpace);
		SkinnedEffect::Ptr()->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));
		SkinnedEffect::Ptr()->SetBoneTransforms(
			&mCharacterInstance.FinalTransforms[0],
			mCharacterInstance.FinalTransforms.size());
		SkinnedEffect::Ptr()->SetWorld(world);
		SkinnedEffect::Ptr()->SetWorldViewProj(worldViewProj);
		SkinnedEffect::Ptr()->SetTexTransform(XMMatrixIdentity());
		SkinnedEffect::Ptr()->SetBoneTransforms(
			&mCharacterInstance.FinalTransforms[0],
			mCharacterInstance.FinalTransforms.size());

		animatedTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		for (UINT subset = 0; subset < mCharacterInstance.Model->SubsetCount; ++subset)
		{
			SkinnedEffect::Ptr()->SetMaterial(mCharacterInstance.Model->Mat[subset]);
			SkinnedEffect::Ptr()->SetDiffuseMap(mCharacterInstance.Model->DiffuseMapSRV[subset]);
			SkinnedEffect::Ptr()->SetNormalMap(mCharacterInstance.Model->NormalMapSRV[subset]);

			animatedTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			mCharacterInstance.Model->ModelMesh.Draw(md3dImmediateContext, subset);
		}
	}

	HR(mSwapChain->Present(0, 0));
}

void AnimatedMeshApp::UpdateScene(float dt)
{
	D3DApp::UpdateScene(dt);
	mCharacterInstance.Update(dt);
}

void AnimatedMeshApp::OnResize()
{
	D3DApp::OnResize();

	mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}