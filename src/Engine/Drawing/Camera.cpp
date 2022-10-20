#include "Camera.h"

#include "Helpers/MathHelper.h"

Camera::Camera(CameraType Type) noexcept :
	m_CartesianTarget(0.0f,0.0f,0.0f),
	m_SphericalTarget(0.0f,0.0f,0.0f),
	m_View(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f),
	m_Proj(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f),
	m_Eye(0.0f, 0.0f, -5.0f),
	m_Right(1.0f, 0.0f, 0.0f),
	m_Up(0.0f, 1.0f, 0.0f),
	m_Look(0.0f, 0.0f, 1.0f),
	m_Target(nullptr)
{
	this->m_CameraType = Type;
	switch (Type) {
	case CameraType::CartesianTarget:

		m_Target = &m_CartesianTarget;
		break;
	case CameraType::SphericalTarget:

		m_Target = &m_SphericalTarget;
		m_SphericalTarget.x = 1.0f; // R should be 1, and stay 1
		break;
	case CameraType::PitchYaw:
		UpdatePitchYawMatrix();
		break;
	default:
	case CameraType::PitchYawRoll:

		break;
	}
}

void Camera::SetTarget(float a, float b, float c)
{
	m_Target->x = a;
	m_Target->y = b;
	m_Target->z = c;
}

// for m_Camera Types CartesianTarget and SphericalTarget
void Camera::AddXTarget(float x) { m_Target->x += x; }
void Camera::AddYTarget(float y) { m_Target->y += y; }
void Camera::AddZTarget(float z) { m_Target->z += z; }
void Camera::AddRTarget(float r) { AddXTarget(r); }
void Camera::AddThetaTarget(float theta) { AddYTarget(theta); }
void Camera::AddPhiTarget(float phi) { AddZTarget(phi); }
float Camera::GetXTarget() { return m_Target->x; }
float Camera::GetYTarget() { return m_Target->y; }
float Camera::GetZTarget() { return m_Target->z; }
void Camera::AddXEye(float x) { m_Eye.x += x; }
void Camera::AddYEye(float y) { m_Eye.y += y; }
void Camera::AddZEye(float z) { m_Eye.z += z; }

// For m_Camera Types CartesianTarget and SphericalTarget
void Camera::SetPosTarget(floaty3 eye, floaty3 target)
{
	m_Eye = eye;
	(*m_Target) = target;
	UpdatePitchYawMatrix();
}

void Camera::SetPosLookUp(floaty3 eye, floaty3 target, floaty3 up)
{
	m_Eye = eye;
	m_Look = target;
	m_Up = up;
	m_Right = m_Look.cross(m_Up);
	UpdatePitchYawMatrix();
}

// For m_Camera Types CartesianTarget and SphericalTarget
void Camera::SetPosTargetCS(floaty3 eyeInCartesian, floaty3 targetInSpherical)
{
	this->m_Eye = eyeInCartesian;
	this->m_SphericalTarget = targetInSpherical;
}

// For m_Camera Types CartesianTarget and SphericalTarget
void Camera::SetPosTargetCC(floaty3 eyeInCartesian, floaty3 targetInCartesian)
{
	this->m_Eye = eyeInCartesian;
	this->m_CartesianTarget = targetInCartesian;
}

// For m_Camera Types CartesianTarget and SphericalTarget
floaty3 Camera::GetTarget()
{
	floaty3 out{ 0.0,0.0,0.0 };
	if (m_CameraType == CameraType::CartesianTarget)
	{
		// Convert cartesian to spherical
		ConvertToSpherical(&m_CartesianTarget, &out);
	}

	if (m_CameraType == CameraType::SphericalTarget)
	{
		// Convert spherical to cartesian
		ConvertToCartesian(&m_SphericalTarget, &out);
	}
	if (!(m_CameraType == CameraType::CartesianTarget || m_CameraType == CameraType::SphericalTarget))
	{
		// some debug thing here because this method shouldn't be called when camera type is not spherical target or cartesian target
	}
	return out;
}

void Camera::UpdateViewMatrix()
{
	switch (m_CameraType)
	{
	case CameraType::CartesianTarget:
	{
		floaty3 target = GetTarget();
		if (target == floaty3{ 0.f, 0.f, 0.f })
			target = { 0.f, 0.f, 1.f };
		m_View = Matrixy4x4::LookAt(m_Eye, target, { 0.f, 1.f, 0.f });
		break;
	}
	case CameraType::SphericalTarget:
	{
		floaty3 target = GetTarget();
		if (target == floaty3{ 0.f, 0.f, 0.f })
			target = { 0.f, 0.f, 1.f };
		m_View = Matrixy4x4::LookAt(m_Eye, target, { 0.f, 1.f, 0.f });
		break;
	}
	case CameraType::PitchYaw:
	{
		UpdatePitchYawMatrix();
		break;
	}
	default:
		break;
	case CameraType::PitchYawRoll:
		break;
	}
}

floaty3 Camera::GetCartesian()
{
	if (m_CameraType == CameraType::CartesianTarget)
	{
		return m_CartesianTarget;
	}

	if (m_CameraType == CameraType::SphericalTarget)
	{
		floaty3 out;
		ConvertToCartesian(&m_SphericalTarget, &out);
		return out;
	}
	// debug checkpoint, camera type wasn't one that was expected
	return { 0.f, 0.f, 0.f };
}

floaty3 Camera::GetSpherical()
{
	if (m_CameraType == CameraType::SphericalTarget)
	{
		return m_SphericalTarget;
	}

	if (m_CameraType == CameraType::CartesianTarget)
	{
		floaty3 out;
		ConvertToSpherical(&m_CartesianTarget, &out);
		return out;
	}
	// debug checkpoint
	return { 0.f, 0.f, 0.f };
}


void ConvertToCartesian(floaty3 *sphericalIn, floaty3 *cartesianOut)
{
	float cx = 0.0f, cy = 0.0f, cz = 0.0f;
	float r = sphericalIn->x, theta = (float)(sphericalIn->y + Math::PiF), phi = sphericalIn->z;
	cz = r * sin(theta) * cos(phi);
	cx = r * sin(theta) * sin(phi);
	cy = r * cos(theta);
	cartesianOut->x = cx;
	cartesianOut->y = cy;
	cartesianOut->z = cz;
}

void ConvertToSpherical(floaty3 *cartesianIn, floaty3 *sphericalOut)
{
	auto& cartMath = *cartesianIn;
	float r = cartMath.magnitude();
	float theta = 0.0f;
	if (r != 0.0f)
	{
		theta = acosf(cartesianIn->z / r);
	}
	float phi = 0.0f;
	if (cartesianIn->x != 0)
	{
		phi = atanf(cartesianIn->y / cartesianIn->x);
	}
	sphericalOut->x = r;
	sphericalOut->y = theta;
	sphericalOut->z = phi;
}

floaty3 Camera::GetPosition() const
{
	return m_Eye;
}

void Camera::SetPosition(float x, float y, float z)
{
	m_Eye.x = x;
	m_Eye.y = y;
	m_Eye.z = z;
}

void Camera::SetPosition(const floaty3 & v)
{
	m_Eye = v;
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	m_FovY = fovY;
	m_Aspect = aspect;
	m_NearZ = zn;
	m_FarZ = zf;
	m_NearWindowHeight = 2.0f * m_NearZ * tanf(0.5f*m_FovY);
	m_FarWindowHeight = 2.0f * m_FarZ * tanf(0.5f*m_FovY);
	m_Proj = Matrixy4x4::PerspectiveFovR(m_FovY, m_Aspect, m_NearZ, m_FarZ);
}
float Camera::GetNearZ() const
{
	return m_NearZ;
}

float Camera::GetFovX() const
{
	float halfWidth = 0.5f*GetNearWindowWidth();
	return 2.0f*atan(halfWidth / m_NearZ);
}
float Camera::GetNearWindowWidth() const
{
	return m_Aspect * m_NearWindowHeight;
}
float Camera::GetNearWindowHeight() const
{
	return m_NearWindowHeight;
}
float Camera::GetFarWindowWidth() const
{
	return m_Aspect * m_FarWindowHeight;
}
float Camera::GetFarWindowHeight() const
{
	return m_FarWindowHeight;
}


void Camera::Walk(float d)
{
	floaty3 s = { d, d, d };
	auto &l = m_Look;
	auto &p = m_Eye;
	m_Eye = { s.x * l.x + p.x, s.y * l.y + p.y, s.z * l.z + p.z };
}

void Camera::Climb(float d)
{
	floaty3 s = { d, d, d };
	auto &l = m_Up;
	auto &p = m_Eye;
	m_Eye = { s.x * l.x + p.x, s.y * l.y + p.y, s.z * l.z + p.z };
}


void Camera::Strafe(float d)
{
	floaty3 s = { d, d, d };
	auto &l = m_Right;
	auto &p = m_Eye;
	m_Eye = { s.x * l.x + p.x, s.y * l.y + p.y, s.z * l.z + p.z };
}

void Camera::Pitch(float angle)
{
	Matrixy3x3 Rot = Matrixy3x3::RotationAxisR(m_Right, angle);
	m_Up = Rot.Transform(m_Up);
	m_Look = Rot.Transform(m_Look);
}
void Camera::RotateY(float angle)
{
	Matrixy3x3 Rot = Matrixy3x3::RotationY(angle);
	m_Right = Rot.Transform(m_Right);
	m_Up = Rot.Transform(m_Up);
	m_Look = Rot.Transform(m_Look);
}

void Camera::ThirdPerson(float dAzimuth, float dPolar)
{
	if (!dAzimuth && !dPolar)
		return;

	floaty3 Translate = { m_Look.x * m_RotDistance + m_Eye.x, m_Look.y * m_RotDistance + m_Eye.y, m_Look.z * m_RotDistance + m_Eye.z };
	m_Eye -= Translate;

	if (dPolar)
		m_Eye = Matrixy4x4::Transform(m_Eye, Matrixy4x4::RotationAxisR(m_Right, dPolar));
	if (dAzimuth)
		m_Eye = Matrixy4x4::Transform(m_Eye, Matrixy4x4::RotationAxisR(m_Up, dAzimuth));

	m_Eye += Translate;

	RotateY(dAzimuth);
	Pitch(dPolar);
}

void Camera::ThirdPersonMove(float move)
{
	m_Eye = { m_Eye.x - m_Look.x * move, m_Eye.y - m_Look.y * move, m_Eye.z - m_Look.z * move };
	m_RotDistance += move;
}

void Camera::ThirdPersonAzimuth(float dAzimuth)
{
	(void)dAzimuth;
}

void Camera::ThirdPersonPolar(float dPolar)
{
	(void)dPolar;
}

Voxel::Frustum Camera::CreateFrustum()
{
	floaty3 lower_left, upper_left, lower_right, upper_right;

	float far_half_height = 0.5f * GetFarWindowHeight(), far_half_width = 0.5f * GetFarWindowWidth();

	floaty3 far_centre = m_Eye + m_Look * m_FarZ;

	upper_right = far_centre + (m_Right * far_half_width) + (m_Up * far_half_height);
	upper_left = far_centre + ((-m_Right) * far_half_width) + (m_Up * far_half_height);
	lower_right = far_centre + (m_Right * far_half_width) + ((-m_Up) * far_half_height);
	lower_left = far_centre + ((-m_Right) * far_half_width) + ((-m_Up) * far_half_height);

	Voxel::Frustum out;

	out.Planes[0].Normal = floaty3::SafelyNormalized((lower_left - m_Eye).cross(upper_left - m_Eye));
	out.Planes[0].Distance = out.Planes[0].Normal.dot(m_Eye);
	out.Planes[1].Normal = floaty3::SafelyNormalized((upper_left - m_Eye).cross(upper_right - m_Eye));
	out.Planes[1].Distance = out.Planes[1].Normal.dot(m_Eye);
	out.Planes[2].Normal = floaty3::SafelyNormalized((upper_right - m_Eye).cross(lower_right - m_Eye));
	out.Planes[2].Distance = out.Planes[2].Normal.dot(m_Eye);
	out.Planes[3].Normal = floaty3::SafelyNormalized((lower_right - m_Eye).cross(lower_left - m_Eye));
	out.Planes[3].Distance = out.Planes[3].Normal.dot(m_Eye);
	out.Planes[4].Normal = m_Look;
	out.Planes[4].Distance = out.Planes[4].Normal.dot(m_Eye + m_Look * m_NearZ);
	out.Planes[5].Normal = -m_Look;
	out.Planes[5].Distance = out.Planes[5].Normal.dot(m_Eye + m_Look * m_FarZ);


#ifdef false
	static constexpr floaty4 HomogenousPoints[6] =
	{
		{ 1.f, 0.f, 1.f, 1.f }, // Right
		{ -1.f, 0.f, 1.f, 1.f }, // Left
		{ 0.f, 1.f, 1.f, 1.f }, // Top
		{ 0.f, -1.f, 1.f, 1.f }, // Bottom
		{ 0.f, 0.f, 0.f, 1.f }, // Near Plane
		{ 0.f, 0.f, 1.f, 1.f }, // Far Plane
	};

	Matrixy4x4 invproj;
	if (!Matrixy4x4::Inverted(m_Proj, invproj))
	{
		DWARNING("Cannot compute frustum with un-inversible projection matrix");
		return out;
	}

	floaty4 Points[6];

	for (size_t i = 6; i-- > 0; )
	{
		Points[i] = invproj.Transform(HomogenousPoints[i]);
	}

	int gotem = 69;

	for (size_t i = 6; i-- > 0; )
		Points[i] = Points[i] / Points[i].w;

	int nice = 69;
#endif

	return out;
}

Voxel::CameraFrustum Camera::CreateOptimizedFrustum()
{
	Voxel::Frustum tmp = CreateFrustum();
	Voxel::CameraFrustum out;
	for (int i = 0; i < 6; ++i)
		out.Planes[i] = tmp.Planes[i];

	float ViewDistance = m_FarZ - m_NearZ;

	floaty3 P = { 0.f, 0.f, m_NearZ + 0.5f * ViewDistance };
	floaty3 Q = { m_FarWindowHeight * m_Aspect * 0.5f, m_FarWindowHeight * 0.5f, ViewDistance };

	floaty3 diff = P - Q;

	out.EncasingSphere.Radius = diff.magnitude();
	out.EncasingSphere.Centre = m_Eye + (m_Look * (ViewDistance * 0.5f + m_NearZ));
	
	return out;
}

void Camera::UpdatePitchYawMatrix()
{
	m_Look.normalize();
	m_Right = floaty3::Normalized(m_Look.cross(m_Up));
	m_Up = m_Right.cross(m_Look);

	float x = -m_Eye.dot(m_Right);
	float y = -m_Eye.dot(m_Up);
	float z = -m_Eye.dot(m_Look);

	m_View.m[0][0] = m_Right.x;
	m_View.m[1][0] = m_Right.y;
	m_View.m[2][0] = m_Right.z;
	m_View.m[3][0] = x;
	m_View.m[0][1] = m_Up.x;
	m_View.m[1][1] = m_Up.y;
	m_View.m[2][1] = m_Up.z;
	m_View.m[3][1] = y;
	m_View.m[0][2] = -m_Look.x;
	m_View.m[1][2] = -m_Look.y;
	m_View.m[2][2] = -m_Look.z;
	m_View.m[3][2] = -z;
	m_View.m[0][3] = 0.0f;
	m_View.m[1][3] = 0.0f;
	m_View.m[2][3] = 0.0f;
	m_View.m[3][3] = 1.0f;
}

void Camera::LookAt(const floaty3 & pos, const floaty3 & target, const floaty3 & up)
{
	m_Look = floaty3::Normalized(target - pos);
	m_Right = floaty3::Normalized(m_Look.cross(up));
	m_Up = m_Right.cross(m_Look);
}

Matrixy4x4 Camera::View() const { return m_View; }
Matrixy4x4 Camera::Proj() const { return m_Proj; }
Matrixy4x4 Camera::ViewProj() const {
	Matrixy4x4 ViewProj = Matrixy4x4::MultiplyE(m_View, m_Proj);
	return ViewProj;
}