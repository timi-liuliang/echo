#include "TransformWidget.h"
#include "EchoEngine.h"
#include <engine/core/geom/Ray.h>

#define TWODIMENSION_SCALE 120

namespace Studio
{
	static float AresTwoLineAngle(const Echo::Vector3& lineFirst, const Echo::Vector3& lineSecond)
	{
		float length = lineFirst.len() * lineSecond.len();
		if (length == 0.0f)
		{
			return 0.0f;
		}

		length = lineFirst.dot(lineSecond) / length;
        length = std::max<float>(-1.0f, length);
        length = std::min<float>(1.0f, length);

		return acos(length);
	}

	//// 在平面上旋转,返回相对旋转矩阵(第一个参数返回角度,)
	//static float RotateOnPlane(const Vector3& planePoint, const Vector3& planeNormal, const Vector3& rayPos0, const Vector3& rayDir0, const Vector3& rayPos1, const Vector3& rayDir1)
	//{
	//	// 根据点与射线确定面
	//	Plane3   rotatePlane(planePoint, planeNormal);
	//	Line3 line0(rayPos0, rayDir0);
	//	Line3 line1(rayPos1, rayDir1);
	//	IntrLine3Plane3 intrLP0(line0, rotatePlane);
	//	IntrLine3Plane3 intrLP1(line1, rotatePlane);
	//	if (!intrLP0.Test() || !intrLP1.Test())
	//		return 0.f;

	//	Vector3 pointBegin = intrLP0.m_intrPoint;
	//	Vector3 pointEnd = intrLP1.m_intrPoint;

	//	// 求旋转角度
	//	Matrix44 pOut;
	//	float tpAngle = AresTwoLineAngle(pointBegin - planePoint, pointEnd - planePoint);
	//	Matrix44RotationAxisAngle(pOut, planeNormal, tpAngle);

	//	// 角度正负
	//	Vector3 normal;
	//	normal = Vector3Cross(planePoint - pointBegin, pointEnd - planePoint);
	//	if (Vector3Dot(normal, planeNormal) < 0.0f)
	//	{
	//		tpAngle = -tpAngle;
	//	}

	//	return tpAngle;
	//}

	TransformWidget::TransformWidget()
	{
		m_fScale = 1.f;

		m_editType = EditType::Translate;
		m_rotateType = RotateType::None;
		m_moveType = MoveType::None;
		m_position = Echo::Vector3::ZERO;

		m_axis = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_axis->setParent(EchoEngine::instance()->getInvisibleEditorNode());
		m_axis->setVisible(true);

		draw();
	}

	void TransformWidget::draw()
	{
		using namespace Echo;

		// color
		const Color White(1.f, 1.f, 1.f, 1.f);
		const String renderType = m_is2d ? "2d" : "3d";
		const real32 scale = m_is2d ? TWODIMENSION_SCALE : 1.f;

		m_axis->clear();
		m_axis->setRenderType(renderType);

		if (m_editType == EditType::Translate)
		{
			if (m_is2d)
			{
				Vector3 position(m_position.x, m_position.y, m_2dDepth);
				m_axis->setWorldPosition(position);

				// axis line
				m_axis->drawLine(Vector3(0.0f, 0.0f, 0.0f) * scale, Vector3(1.0f, 0.0f, 0.0f) * scale, isMoveType(MoveType::XAxis) ? White : Echo::Color::RED);
				m_axis->drawLine(Vector3(0.0f, 0.0f, 0.0f) * scale, Vector3(0.0f, 1.0f, 0.0f) * scale, isMoveType(MoveType::YAxis) ? White : Echo::Color::GREEN);

				// plane
				m_axis->drawLine(Vector3(0.4f, 0.0f, 0.0f) * scale, Vector3(0.4f, 0.4f, 0.0f) * scale, isMoveType(MoveType::XYPlane) ? White : Color::RED);
				m_axis->drawLine(Vector3(0.4f, 0.4f, 0.0f) * scale, Vector3(0.0f, 0.4f, 0.0f) * scale, isMoveType(MoveType::XYPlane) ? White : Color::GREEN);

				// cones
				drawCone(0.08f * scale, 0.4f * scale, Transform(Vector3::UNIT_X * scale, Vector3::ONE, Quaternion::IDENTITY), isMoveType(MoveType::XAxis) ? White : Color::RED);
				drawCone(0.08f * scale, 0.4f * scale, Transform(Vector3::UNIT_Y * scale, Vector3::ONE, Quaternion::fromVec3ToVec3(Echo::Vector3::UNIT_X, Echo::Vector3::UNIT_Y)), isMoveType(MoveType::YAxis) ? White : Echo::Color::GREEN);
			}
			else
			{
				m_axis->setWorldPosition(m_position);

				// axis line
				m_axis->drawLine(Vector3(0.0f, 0.0f, 0.0f) * scale, Vector3(1.0f, 0.0f, 0.0f) * scale, isMoveType(MoveType::XAxis) ? White : Echo::Color::RED);
				m_axis->drawLine(Vector3(0.0f, 0.0f, 0.0f) * scale, Vector3(0.0f, 1.0f, 0.0f) * scale, isMoveType(MoveType::YAxis) ? White : Echo::Color::GREEN);
				m_axis->drawLine(Vector3(0.0f, 0.0f, 0.0f) * scale, Vector3(0.0f, 0.0f, 1.0f) * scale, isMoveType(MoveType::ZAxis) ? White : Echo::Color::BLUE);

				// plane
				m_axis->drawLine(Vector3(0.4f, 0.0f, 0.0f) * scale, Vector3(0.4f, 0.4f, 0.0f) * scale, isMoveType(MoveType::XYPlane) ? White : Color::RED);
				m_axis->drawLine(Vector3(0.4f, 0.4f, 0.0f) * scale, Vector3(0.0f, 0.4f, 0.0f) * scale, isMoveType(MoveType::XYPlane) ? White : Color::GREEN);
				m_axis->drawLine(Vector3(0.0f, 0.4f, 0.0f) * scale, Vector3(0.0f, 0.4f, 0.4f) * scale, isMoveType(MoveType::YZPlane) ? White : Color::GREEN);
				m_axis->drawLine(Vector3(0.0f, 0.4f, 0.4f) * scale, Vector3(0.0f, 0.0f, 0.4f) * scale, isMoveType(MoveType::YZPlane) ? White : Color::BLUE);
				m_axis->drawLine(Vector3(0.4f, 0.0f, 0.4f) * scale, Vector3(0.0f, 0.0f, 0.4f) * scale, isMoveType(MoveType::XZPlane) ? White : Color::BLUE);
				m_axis->drawLine(Vector3(0.4f, 0.0f, 0.0f) * scale, Vector3(0.4f, 0.0f, 0.4f) * scale, isMoveType(MoveType::XZPlane) ? White : Color::RED);

				// cones
				drawCone(0.08f * scale, 0.4f * scale, Transform(Vector3::UNIT_X * scale, Vector3::ONE, Quaternion::IDENTITY), isMoveType(MoveType::XAxis) ? White : Color::RED);
				drawCone(0.08f * scale, 0.4f * scale, Transform(Vector3::UNIT_Y * scale, Vector3::ONE, Quaternion::fromVec3ToVec3(Echo::Vector3::UNIT_X, Echo::Vector3::UNIT_Y)), isMoveType(MoveType::YAxis) ? White : Echo::Color::GREEN);
				drawCone(0.08f * scale, 0.4f * scale, Transform(Vector3::UNIT_Z * scale, Vector3::ONE, Quaternion::fromVec3ToVec3(Echo::Vector3::UNIT_X, Echo::Vector3::UNIT_Z)), isMoveType(MoveType::ZAxis) ? White : Echo::Color::BLUE);
			}
		}
		else if (m_editType == EditType::Scale)
		{
			if (m_is2d)
			{
				Vector3 position(m_position.x, m_position.y, -255.f);
				m_axis->setWorldPosition(position);

				// axis line
				m_axis->drawLine(Vector3(0.0f, 0.0f, 0.0f) * scale, Vector3(1.0f, 0.0f, 0.0f) * scale, isScaleType(ScaleType::XAxis) ? White : Echo::Color::RED);
				m_axis->drawLine(Vector3(0.0f, 0.0f, 0.0f) * scale, Vector3(0.0f, 1.0f, 0.0f) * scale, isScaleType(ScaleType::YAxis) ? White : Echo::Color::GREEN);

				// plane
				m_axis->drawLine(Vector3(0.4f, 0.0f, 0.0f) * scale, Vector3(0.0f, 0.4f, 0.0f) * scale, isScaleType(ScaleType::All) ? White : Color::YELLOW);

				// cones
				drawBox(Echo::Vector3(0.06f, 0.06f, 0.06f) * scale, Transform(Vector3::UNIT_X * scale, Vector3::ONE, Quaternion::IDENTITY), isScaleType(ScaleType::XAxis) ? White : Color::RED);
				drawBox(Echo::Vector3(0.06f, 0.06f, 0.06f) * scale, Transform(Vector3::UNIT_Y * scale, Vector3::ONE, Quaternion::fromVec3ToVec3(Echo::Vector3::UNIT_X, Echo::Vector3::UNIT_Y)), isScaleType(ScaleType::YAxis) ? White : Echo::Color::GREEN);
			}
			else
			{
				m_axis->setWorldPosition(m_position);

				// axis line
				m_axis->drawLine(Vector3(0.0f, 0.0f, 0.0f) * scale, Vector3(1.0f, 0.0f, 0.0f) * scale, isScaleType(ScaleType::XAxis) ? White : Echo::Color::RED);
				m_axis->drawLine(Vector3(0.0f, 0.0f, 0.0f) * scale, Vector3(0.0f, 1.0f, 0.0f) * scale, isScaleType(ScaleType::YAxis) ? White : Echo::Color::GREEN);
				m_axis->drawLine(Vector3(0.0f, 0.0f, 0.0f) * scale, Vector3(0.0f, 0.0f, 1.0f) * scale, isScaleType(ScaleType::ZAxis) ? White : Echo::Color::BLUE);

				// plane
				m_axis->drawLine(Vector3(0.4f, 0.0f, 0.0f) * scale, Vector3(0.0f, 0.4f, 0.0f) * scale, isScaleType(ScaleType::All) ? White : Color::YELLOW);
				m_axis->drawLine(Vector3(0.0f, 0.4f, 0.0f) * scale, Vector3(0.0f, 0.0f, 0.4f) * scale, isScaleType(ScaleType::All) ? White : Color::YELLOW);
				m_axis->drawLine(Vector3(0.4f, 0.0f, 0.0f) * scale, Vector3(0.0f, 0.0f, 0.4f) * scale, isScaleType(ScaleType::All) ? White : Color::YELLOW);

				// boxes
				drawBox(Echo::Vector3(0.06f, 0.06f, 0.06f) * scale, Transform(Vector3::UNIT_X * scale, Vector3::ONE, Quaternion::IDENTITY), isScaleType(ScaleType::XAxis) ? White : Color::RED);
				drawBox(Echo::Vector3(0.06f, 0.06f, 0.06f) * scale, Transform(Vector3::UNIT_Y * scale, Vector3::ONE, Quaternion::fromVec3ToVec3(Echo::Vector3::UNIT_X, Echo::Vector3::UNIT_Y)), isScaleType(ScaleType::YAxis) ? White : Echo::Color::GREEN);
				drawBox(Echo::Vector3(0.06f, 0.06f, 0.06f) * scale, Transform(Vector3::UNIT_Z * scale, Vector3::ONE, Quaternion::fromVec3ToVec3(Echo::Vector3::UNIT_X, Echo::Vector3::UNIT_Z)), isScaleType(ScaleType::ZAxis) ? White : Echo::Color::BLUE);
			}
		}

		//// 三个圆
		//for (int i = 0; i < 3; i++)
		//{
		//	m_pCycle[i] = visualShapeMgr.CreateCycle(5);
		//	m_pCycle[i]->SetRadius(1.0f);
		//	m_pCycle[i]->SetFace(m_vAxisDir[i]);
		//	m_pCycle[i]->SetVisible(false);
		//	//m_pCycle[i]->SetSizeFixed( true);
		//}

		//m_pCycle[0]->SetColor(0xFFFF0000);
		//m_pCycle[1]->SetColor(0xFF00FF00);
		//m_pCycle[2]->SetColor(0xFF0000FF);

		//// 缩放
		//m_pScale = visualShapeMgr.CreateVisualShape(5);
		//Vector3 positions[] =
		//{
		//	Vector3(0.5f, 0.0f, 0.0f),
		//	Vector3(0.0f, 0.5f, 0.0f),
		//	Vector3(0.0f, 0.0f, 0.5f),
		//};

		//WORD  indices[] = { 0, 1, 2 };

		//m_pScale->Set(RenderLayout::TT_TriangleList, positions, sizeof(positions), indices, sizeof(indices), EF_R16UI);
		//m_pScale->SetColor(0xFF33AAFF);
		//m_pScale->SetVisible(false);
	}

	void TransformWidget::drawCone(float radius, float height, const Echo::Transform& transform, const Echo::Color& color)
	{
		// init positions
		Echo::Vector3 positions[27];
		for (int i = 0; i < 25; i++)
		{
			float theta = (2 * Echo::Math::PI * i) / 24.0f;

			positions[i].x = 0.0f;
			positions[i].y = sinf(theta) * radius;
			positions[i].z = cosf(theta) * radius;
		}
		positions[25] = Echo::Vector3(0.0f, 0.0f, 0.0f);
		positions[26] = Echo::Vector3(height, 0.0f, 0.0f);

		// transform
		for (int i = 0; i < 27; i++)
		{
			positions[i] = transform.transformVec3(positions[i]);
		}

		// draw triangles
		for (int i = 0; i < 25; i++)
		{
			float theta = (2 * Echo::Math::PI * i) / 24.0f;

			m_axis->drawTriangle(positions[i], positions[i + 1], positions[25], color);
			m_axis->drawTriangle(positions[i], positions[26], positions[i+1], color);
		}
	}

	void TransformWidget::drawBox(const Echo::Vector3& extent, const Echo::Transform& transform, const Echo::Color& color)
	{
		Echo::Vector3 positions[8];
		Echo::Box3	  box3( Echo::Vector3::ZERO, Echo::Vector3::UNIT_X, Echo::Vector3::UNIT_Y, Echo::Vector3::UNIT_Z, extent.x, extent.y, extent.z);
		box3.buildEightPoints(positions);

		for (int i = 0; i < 8; i++)
		{
			positions[i] = transform.transformVec3(positions[i]);
		}

		// draw triangles
		m_axis->drawTriangle(positions[0], positions[1], positions[2], color);
		m_axis->drawTriangle(positions[0], positions[2], positions[3], color);

		m_axis->drawTriangle(positions[4], positions[5], positions[6], color);
		m_axis->drawTriangle(positions[4], positions[6], positions[7], color);

		m_axis->drawTriangle(positions[0], positions[1], positions[5], color);
		m_axis->drawTriangle(positions[0], positions[5], positions[4], color);

		m_axis->drawTriangle(positions[3], positions[2], positions[6], color);
		m_axis->drawTriangle(positions[3], positions[6], positions[7], color);

		m_axis->drawTriangle(positions[0], positions[3], positions[7], color);
		m_axis->drawTriangle(positions[0], positions[7], positions[4], color);

		m_axis->drawTriangle(positions[1], positions[2], positions[6], color);
		m_axis->drawTriangle(positions[1], positions[6], positions[5], color);
	}

	void TransformWidget::onMouseMove(const Echo::Vector2& localPos)
	{
		Echo::Camera* camera = (Echo::Render::getRenderTypes() & Echo::Render::Type::Type_3D) ? Echo::NodeTree::instance()->get3dCamera() : Echo::NodeTree::instance()->get2dCamera();
		if (camera && m_isVisible)
		{
			// get camera ray
			Echo::Ray ray0;
			Echo::Ray ray1;
			camera->getCameraRay(ray0, m_mousePos);
			camera->getCameraRay(ray1, localPos);

			switch (m_editType)
			{
			case EditType::Translate:
			{
				float       fDist = 0.0f;
				Echo::Vector3 relaTrans;
				if (m_is2d)
				{
					switch (m_moveType)
					{
					case MoveType::XAxis:
					{
						Echo::Plane plane(m_position, Echo::Vector3::UNIT_Z);
						translateOnPlane(&relaTrans, plane, ray0, ray1);
						relaTrans.y = 0.f;
					}
					break;
					case MoveType::YAxis:
					{
						Echo::Plane plane(m_position, Echo::Vector3::UNIT_Z);
						translateOnPlane(&relaTrans, plane, ray0, ray1);
						relaTrans.x = 0.f;
					}
					break;
					case MoveType::XYPlane:
					{
						Echo::Plane plane(m_position, Echo::Vector3::UNIT_Z);
						translateOnPlane(&relaTrans, plane, ray0, ray1);
					}
					break;
					default:
					{
						return;
					}
					}
				}
				else
				{
					switch (m_moveType)
					{
					case MoveType::XAxis:
					{
						fDist = translateOnAxis(ray0, ray1, m_position, Echo::Vector3::UNIT_X);
						relaTrans = fDist * Echo::Vector3::UNIT_X;
					}
					break;
					case MoveType::YAxis:
					{
						fDist = translateOnAxis(ray0, ray1, m_position, Echo::Vector3::UNIT_Y);
						relaTrans = fDist * Echo::Vector3::UNIT_Y;
					}
					break;
					case MoveType::ZAxis:
					{
						fDist = translateOnAxis(ray0, ray1, m_position, Echo::Vector3::UNIT_Z);
						relaTrans = fDist * Echo::Vector3::UNIT_Z;
					}
					break;
					case MoveType::XYPlane:
					{
						Echo::Plane plane(m_position, Echo::Vector3::UNIT_Z);
						translateOnPlane(&relaTrans, plane, ray0, ray1);
					}
					break;
					case MoveType::YZPlane:
					{
						Echo::Plane plane(m_position, Echo::Vector3::UNIT_X);
						translateOnPlane(&relaTrans, plane, ray0, ray1);
					}
					break;
					case MoveType::XZPlane:
					{
						Echo::Plane plane(m_position, Echo::Vector3::UNIT_Y);
						translateOnPlane(&relaTrans, plane, ray0, ray1);
					}
					break;
					default:
					{
						return;
					}
					}
				}

				onTranslate(relaTrans);
			}
			break;
	//		case EM_EDIT_ROTATE:
	//		{
	//			float fAngle0 = 0.0f;
	//			float fAngle1 = 0.0f;
	//			float fAngle2 = 0.0f;

	//			switch (m_rotateType)
	//			{
	//			case EM_ROTATE_X:
	//			{
	//				fAngle0 = RotateOnPlane(m_vPosition, m_vAxisDir[0], rayOrig0, rayDir0, rayOrig1, rayDir1);
	//			}
	//			break;
	//			case EM_ROTATE_Y:
	//			{
	//				fAngle1 = RotateOnPlane(m_vPosition, m_vAxisDir[1], rayOrig0, rayDir0, rayOrig1, rayDir1);
	//			}
	//			break;
	//			case EM_ROTATE_Z:
	//			{
	//				fAngle2 = RotateOnPlane(m_vPosition, m_vAxisDir[2], rayOrig0, rayDir0, rayOrig1, rayDir1);
	//			}
	//			break;
	//			}

	//			// 旋转所有模型
	//			for (size_t i = 0; i < m_entityList.size(); i++)
	//			{
	//				Transform tranform = m_entityList[i]->GetTransform();
	//				tranform.AddRotation(m_vAxisDir[0], fAngle0);
	//				tranform.AddRotation(m_vAxisDir[1], fAngle1);
	//				tranform.AddRotation(m_vAxisDir[2], fAngle2);

	//				m_entityList[i]->SetTransform(tranform);
	//			}

	//			// 旋转所有模型
	//			for (size_t i = 0; i < m_transforms.size(); i++)
	//			{
	//				m_transforms[i]->AddRotation(m_vAxisDir[0], fAngle0);
	//				m_transforms[i]->AddRotation(m_vAxisDir[1], fAngle1);
	//				m_transforms[i]->AddRotation(m_vAxisDir[2], fAngle2);
	//			}
	//		}
	//		break;

	//		case EM_EDIT_SCALE:
	//		{
	//			// 缩放所有模型
	//			for (size_t i = 0; i < m_entityList.size(); i++)
	//			{
	//				Transform tranform = m_entityList[i]->GetTransform();
	//				float scale = tranform.GetScale().x - 0.002f * (ptPre->y - ptCurr->y);
	//				tranform.SetScale(scale);

	//				m_entityList[i]->SetTransform(tranform);
	//			}

	//			// 旋转所有模型
	//			for (size_t i = 0; i < m_transforms.size(); i++)
	//			{
	//				float scale = m_transforms[i]->GetScale().x - 0.002f * (ptPre->y - ptCurr->y);
	//				m_transforms[i]->SetScale(scale);
	//			}
	//		}
	//		break;
            default: break;
			}
		}

		m_mousePos = localPos;
	}

	void TransformWidget::updateTranslateCollisionBox()
	{
		using namespace Echo;

		if (m_is2d)
		{
			const Echo::real32 scale = TWODIMENSION_SCALE;

			m_moveBoxs[int(MoveType::XAxis)].set(m_position + Vector3(1.f, 0.f, 0.f) * scale, Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z, 0.60f * scale, 0.15f * scale, 0.15f * scale);
			m_moveBoxs[int(MoveType::YAxis)].set(m_position + Vector3(0.f, 1.f, 0.f) * scale, Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z, 0.15f * scale, 0.60f * scale, 0.15f * scale);
			m_moveBoxs[int(MoveType::XYPlane)].set(m_position + Vector3(0.2f, 0.2f, 0.0f) * scale, Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z, 0.2f * scale, 0.2f * scale, 0.05f * scale);
		}
		else
		{
			const Echo::real32 scale = 1.f;

			m_moveBoxs[int(MoveType::XAxis)].set(m_position + Vector3(1.f, 0.f, 0.f) * scale, Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z, 0.60f * scale, 0.15f * scale, 0.15f * scale);
			m_moveBoxs[int(MoveType::YAxis)].set(m_position + Vector3(0.f, 1.f, 0.f) * scale, Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z, 0.15f * scale, 0.60f * scale, 0.15f * scale);
			m_moveBoxs[int(MoveType::ZAxis)].set(m_position + Vector3(0.f, 0.f, 1.f) * scale, Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z, 0.15f * scale, 0.15f * scale, 0.60f * scale);
			m_moveBoxs[int(MoveType::XYPlane)].set(m_position + Vector3(0.2f, 0.2f, 0.0f) * scale, Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z, 0.2f * scale, 0.2f * scale, 0.05f * scale);
			m_moveBoxs[int(MoveType::YZPlane)].set(m_position + Vector3(0.0f, 0.2f, 0.2f) * scale, Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z, 0.05f * scale, 0.2f * scale, 0.2f * scale);
			m_moveBoxs[int(MoveType::XZPlane)].set(m_position + Vector3(0.2f, 0.0f, 0.2f) * scale, Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z, 0.2f * scale, 0.05f * scale, 0.2f * scale);
		}
	}

	bool TransformWidget::onMouseDown(const Echo::Vector2& localPos)
	{
		m_mousePos = localPos;

		Echo::Camera* camera = (Echo::Render::getRenderTypes() & Echo::Render::Type::Type_3D) ? Echo::NodeTree::instance()->get3dCamera() : Echo::NodeTree::instance()->get2dCamera();
		if (camera)
		{
			// get camera ray
			Echo::Ray ray;
			camera->getCameraRay(ray, localPos);

			// update collision box
			updateTranslateCollisionBox();

			// hit operate
			if (m_isVisible)
			{
				switch (m_editType)
				{
				case EditType::Translate:
				{
					m_moveType = MoveType::None;
					for (int i = int(MoveType::XAxis); i <= int(MoveType::XZPlane); i++)
					{
						if (ray.hitBox3(m_moveBoxs[i]))
						{
							m_moveType = MoveType(i);
							break;
						}
					}

					// update display
					draw();
				}
				break;
                        
                case EditType::Rotate: {} break;
				//case EM_EDIT_ROTATE:
				//{
				//	float fLastDist = 1e30f;
				//	float fDist;
				//	Vector3 intersectPos;
				//	Plane3 plane;
				//	m_rotateType = EM_ROTATE_NULL;

				//	// 选择
				//	for (int i = 0; i < 3; i++)
				//	{
				//		Line3 ray(rayOrig, rayDir);
				//		Plane3 plane(m_vPosition, m_vAxisDir[i]);
				//		IntrLine3Plane3 intrLP(ray, plane);
				//		if (intrLP.Test())
				//		{
				//			fDist = intrLP.m_distance;
				//			intersectPos = intrLP.m_intrPoint;
				//			if ((intersectPos - m_vPosition).Length() > 0.8f*m_fScale && (intersectPos - m_vPosition).Length() < 1.2f*m_fScale)
				//			{
				//				if (i == 0) m_rotateType = EM_ROTATE_X;
				//				if (i == 1) m_rotateType = EM_ROTATE_Y;
				//				if (i == 2) m_rotateType = EM_ROTATE_Z;

				//				fLastDist = fDist;

				//				break;
				//			}
				//		}
				//	}

				//	// 显示
				//	switch (m_rotateType)
				//	{
				//	case EM_ROTATE_X:
				//		m_pCycle[0]->SetColor(0xFFFFFFFF);
				//		break;
				//	case EM_ROTATE_Y:
				//		m_pCycle[1]->SetColor(0xFFFFFFFF);
				//		break;
				//	case EM_ROTATE_Z:
				//		m_pCycle[2]->SetColor(0xFFFFFFFF);
				//		break;
				//	}
				//}
				//break;
				case EditType::Scale:
				{
				//	Triangle3 triangle(m_vPosition, m_vPosition, m_vPosition);
				//	triangle.m_v[0].x += 0.5f*m_fScale; triangle.m_v[1].y += 0.5f*m_fScale; triangle.m_v[2].z += 0.5f*m_fScale;
				//	Line3 line3(rayOrig, rayDir);
				//	float fdist;

				//	if (Intersect(line3, triangle, fdist))
				//	{
				//		m_pScale->SetColor(0xFFFFFFFF);
				//	}

					draw();
				}
				break;
				}
			}
		}

		return false;
	}

	void TransformWidget::onMouseUp()
	{
		m_moveType = MoveType::None;
		m_rotateType = RotateType::None;

		draw();
	}

	void TransformWidget::setPosition(const Echo::Vector3& pos)
	{
		m_position = pos;

		updateTranslateCollisionBox();
		draw();
	}

	void TransformWidget::setVisible(bool visible)
	{
		m_axis->setVisible(visible);
	}

	void TransformWidget::setRenderType2d(bool is2d)
	{ 
		m_is2d = is2d;

		updateTranslateCollisionBox();
		draw();
	}

	void  TransformWidget::onTranslate(const Echo::Vector3& trans)
	{
		if (trans != Echo::Vector3::ZERO)
		{
			setPosition(m_position + trans);

			if (m_listener)
			{
				m_listener->onTranslate(trans);
			}
		}
	}

	void TransformWidget::SetEditType(EditType type)
	{
		this->setVisible(false);
		m_editType = type;
		this->setVisible(true);
	}

	void  TransformWidget::setScale(float fScale)
	{
		/*
		m_fScale = fScale;
		for (int i = 0; i < 3; i++)
		{
			//m_pAxes[i]->GetTransform()->SetScale( fScale);
			m_pPlaneLine[i]->GetTransform()->SetScale( fScale);
			m_pPlaneLine[3+i]->GetTransform()->SetScale( fScale);
			m_pCone[i]->GetTransform()->SetScale( fScale);
			m_pCycle[i]->GetTransform()->SetScale( fScale);
		}

		// 设置位置
		m_pCone[0]->GetTransform()->SetTrans(m_fScale + m_vPosition.x, m_vPosition.y, m_vPosition.z);
		m_pCone[1]->GetTransform()->SetTrans(m_vPosition.x, m_fScale + m_vPosition.y, m_vPosition.z);
		m_pCone[2]->GetTransform()->SetTrans(m_vPosition.x, m_vPosition.y, m_fScale + m_vPosition.z);
		*/
	}

	float TransformWidget::translateOnAxis(const Echo::Ray& ray0, const Echo::Ray& ray1, const Echo::Vector3& entityPos, const Echo::Vector3& translateAxis)
	{
		float result = 0.0f;

		// calculate face normal
		Echo::Vector3 planeNormal;
		planeNormal = ray0.m_dir.cross(ray1.m_dir);
		planeNormal.normalize();
		float test = planeNormal.dot(translateAxis) / (planeNormal.len() * translateAxis.len());

		// check
		if (test > 0.3f || test < -0.3f)
			return result;

		planeNormal = planeNormal.cross(translateAxis);
		planeNormal.normalize();

		// check
		if (!planeNormal.len())
			return result;

		// define plane
		Echo::Plane axisPlane(entityPos, planeNormal);
		axisPlane.normalize();

		// intersection
		float hitDistance0;
		float hitDistance1;
		Echo::Ray::HitInfo hitResult0;
		Echo::Ray::HitInfo hitResult1;

		if (!ray0.hitPlane(axisPlane, hitDistance0, hitResult0) || !ray1.hitPlane(axisPlane, hitDistance1, hitResult1))
			return result;

		Echo::Vector3 pointBegin = hitResult0.hitPos;
		Echo::Vector3 pointEnd = hitResult1.hitPos;

		// calculate distance on the axis
		Echo::Vector3 rayMove = pointEnd - pointBegin;
		result = rayMove.dot(translateAxis) / translateAxis.len();

		return result;
	}

	Echo::Vector3* TransformWidget::translateOnPlane(Echo::Vector3* pOut, const Echo::Plane& plane, const Echo::Ray& ray0, const Echo::Ray& ray1)
	{
		// intersection
		float hitDistance0;
		float hitDistance1;
		Echo::Ray::HitInfo hitResult0;
		Echo::Ray::HitInfo hitResult1;
		if (!ray0.hitPlane(plane, hitDistance0, hitResult0) || !ray1.hitPlane(plane, hitDistance1, hitResult1))
		{
			*pOut = Echo::Vector3::ZERO;
			return nullptr;
		}

		Echo::Vector3 pointBegin = hitResult0.hitPos;
		Echo::Vector3 pointEnd = hitResult1.hitPos;

		*pOut = pointEnd - pointBegin;
		return pOut;
	}
}
