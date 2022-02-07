#pragma once

#include "Frustum.h"

#include "Helpers/VectorHelper.h"

enum CameraType {
	CartesianTarget = 1,
	SphericalTarget = 2,
	PitchYaw = 3,
	PitchYawRoll = 4
};

constexpr float IdealNearPlane = 0.1f;
constexpr float IdealFarPlane = 1000.f;

class Camera
{
private:
	// m_Camera coordinate system with coordinates relative to world space.
	// Cache frustum properties.
	float m_NearZ;
	float m_FarZ;
	float m_Aspect;
	float m_FovY;
	float m_NearWindowHeight;
	float m_FarWindowHeight;
	float m_Yaw;
	// Cache View/Proj matrices.
	Matrixy4x4 m_View;
	Matrixy4x4 m_Proj;

	// For CartesianTarget and SphericalTarget m_Camera types
	floaty3 m_SphericalTarget;
	floaty3 m_CartesianTarget;
	floaty3 *m_Target;

	floaty3 m_Eye;
	floaty3 m_Right; 	// view space x-axis (in world coords)
	floaty3 m_Up; 		// view space y-axis (in world coords)
	floaty3 m_Look; 	// view space z-axis (in world coords), left handed
	float m_RotDistance = 5.f; // How Far in front the Third person rotation point is
	CameraType m_CameraType;

public:
	// For CartesianTarget and SphericalTarget m_Camera types
	void SetTarget(float a, float b, float c);
	void AddXTarget(float x); // In spherical is 'r'
	void AddYTarget(float y); // In spherical is 'theta'
	void AddZTarget(float z); // In spherical is 'phi'
	void AddRTarget(float r);
	void AddThetaTarget(float theta);
	void AddPhiTarget(float phi);
	float GetXTarget();
	float GetYTarget();
	float GetZTarget();
	void AddXEye(float x);
	void AddYEye(float y);
	void AddZEye(float z);
	void SetPosTarget(floaty3 eye, floaty3 target);
	void SetPosLookUp(floaty3 eye, floaty3 target, floaty3 up);
	void SetPosTargetCS(floaty3 eyeInCartesian, floaty3 targetInSpherical);
	void SetPosTargetCC(floaty3 eyeInCartesian, floaty3 targetInCartesian);
	floaty3 GetTarget();

	// After modifying camera position/orientation, call
	// to rebuild the view matrix once per frame.
	void UpdateViewMatrix();
	void UpdatePitchYawMatrix();

	floaty3 GetSpherical();
	floaty3 GetCartesian();

	Camera(CameraType Type = PitchYaw) noexcept;
	
	// For PitchYaw/Roll m_Camera types
	// Get/Set world camera position.
	floaty3 GetPosition() 	const;
	void SetPosition(float x, float y, float z);
	void SetPosition(const floaty3& v);
	// Get camera basis vectors.
	inline floaty3 GetRight() 	const { return m_Right; }
	inline floaty3 GetUp() 	const { return m_Up; }
	inline floaty3 GetLook()	const { return m_Look; }
	// Get frustum properties.
	float GetNearZ() 		const;
	inline float GetFarZ() 		const { return m_FarZ; }
	inline float GetAspect()	const { return m_Aspect; }
	inline float GetFovY() 		const { return m_FovY; }
	float GetFovX() 		const;
	// Get near and far plane dimensions in view space coordinates.
	float GetNearWindowWidth() 	const;
	float GetNearWindowHeight() const;
	float GetFarWindowWidth() 	const;
	float GetFarWindowHeight() 	const;
	// Set frustum.
	void SetLens(float fovY, float aspect, float zn, float zf);
	// Define camera space via LookAt parameters.
	void LookAt(const floaty3& pos, const floaty3& target, const floaty3& up);
	// Get View/Proj matrices.
	Matrixy4x4 View() 	const;
	Matrixy4x4 Proj() 	const;
	Matrixy4x4 ViewProj() const;
	// Strafe/Walk the camera a distance d.
	void Strafe(float d);
	void Walk(float d);
	void Climb(float d);
	// Rotate the camera.
	void Pitch(float angle);
	void RotateY(float angle);

	// Third person shit
	void ThirdPerson(float dAzimuth, float dPolar);
	void ThirdPersonMove(float move);
	void ThirdPersonAzimuth(float dAzimuth);
	void ThirdPersonPolar(float dPolar);
	inline float GetThirdPersonDistance() const { return m_RotDistance; }

	Voxel::Frustum CreateFrustum();
	Voxel::CameraFrustum CreateOptimizedFrustum();
};

void ConvertToCartesian(floaty3 *sphericalIn, floaty3 *cartesianOut);
void ConvertToSpherical(floaty3 *cartesianIn, floaty3 *sphericalOut);
