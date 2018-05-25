#include <gtest/gtest.h>
#include <engine/core/camera/Camera.h>

TEST(OrthoCamera, depthIsLinear)
{
	//Echo::Vector3 vCam2DPos(0, 0, 0);
	//Echo::Vector3 vCam2DDir = -Echo::Vector3::UNIT_Z;

	//Echo::Camera orthoCamera(Echo::Camera::PM_ORTHO);
	//orthoCamera.setPosition(vCam2DPos);
	//orthoCamera.setDirection(vCam2DDir);
	//orthoCamera.setNearClip(-256.f);
	//orthoCamera.setFarClip(256.0f);
	//orthoCamera.update();

	//for (int i = 0; i < 1000; i++)
	//{
	//	float lenght = i / 1000.f * 512.f;
	//	Echo::Vector3 pos = orthoCamera.getPosition() + orthoCamera.getDirection() * lenght;

	//	orthoCamera.getViewProjMatrix();
	//	Echo::Vector4 outPos;
	//	Echo::Matrix4::TransformVec4(outPos, Echo::Vector4(pos.x, pos.y, pos.z, 1.f), orthoCamera.getViewProjMatrix());

	//	outPos = outPos / outPos.w;

	//	float u = outPos.x;
	//	float v = outPos.y;
	//	float depth = outPos.z;

	//	float normailzedLength = i / 1000.f;

	//	// find the relation between "normalizedLength" with "depth"
	//	int a = 10;
	//}
}