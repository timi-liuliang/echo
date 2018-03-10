#include "engine/core/Math/Quaternion.h"
#include "engine/core/Math/Vector4.h"
#include "engine/core/Util/Singleton.h"
#include "Bone.h"
#include "Animation.h"
#include "AnimBlender.h"
#include "AnimState.h"
#include "AnimManager.h"
#include "SkeletonManager.h"
#include "AnimSystem.h"
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>
#include <rapidxml/rapidxml_utils.hpp>

using namespace rapidxml;

namespace Echo
{
	// 构造函数
	AnimSystem::AnimSystem(const String& name, bool isManual)
		: Resource(name, RT_AnimationSystem, isManual)
		, m_skeleton(NULL)
		, m_blender(NULL)
		, m_blenderExternal(NULL)
		, m_isPause(false)
	{
	}

	// 析构函数
	AnimSystem::~AnimSystem()
	{
		if (m_blender)
			AnimManager::instance()->destroyAnimBlenders(&m_blender, 1);

		// 清空动画
		for (size_t i = 0; i < m_animations.size(); i++)
			AnimManager::instance()->releaseResource(m_animations[i]);
		m_animations.clear();

		EchoSafeDeleteContainer(m_animStates, AnimState);

		if (m_skeleton)
		{
			SkeletonManager::instance()->releaseResource(m_skeleton);
		}
	}

	// 更新
	void AnimSystem::tick(i32 elapsedTime, bool isSync)
	{
		if (!m_isPause)
		{
			if (getBlender())
				getBlender()->frameMove(elapsedTime, isSync);
		}
	}

	// 播放动画
	void AnimSystem::playAnimation(AnimState* animState, ui32 blendTime)
	{
		if (getBlender() && animState && animState->getLength() > 0)
		{
			getBlender()->blend(animState, blendTime);
		}
	}

	// 获取指定骨骼当前位置
	Vector3 AnimSystem::getBonePos(const Bone* bone)
	{
		return getBlender()->getAnimPosition(bone->getId());
	}

	// 计算内存占用大小
	size_t AnimSystem::calculateSize() const
	{
		// 未实现
		return 0;
	}

	// 资源加载
	bool AnimSystem::loadImpl()
	{
		try
		{
			MemoryReader memReader(mName.c_str());
			xml_document<> doc;
			doc.parse<0>(memReader.getData<char*>());

			// 获取主节点
			xml_node<>* rootnode = doc.first_node("animsys"); EchoAssert(rootnode);
			if (rootnode)
			{
				// 骨架信息
				xml_attribute<>* skeletonAtt = rootnode->first_attribute("skeleton");
				if (skeletonAtt)
				{
					String skeleton = skeletonAtt->value();
					if (!skeleton.empty())
					{
						setSkeleton(skeleton.c_str());
					}
				}

				// 动画信息
				xml_attribute<>* animationsAtt = rootnode->first_attribute("anims");
				if (animationsAtt)
				{
					StringArray anims = StringUtil::Split(animationsAtt->value(), ";");
					for (size_t i = 0; i < anims.size(); i++)
					{
						addAnimation(anims[i].c_str());
					}
				}
			}
		}
		catch (...)
		{
			EchoLogError("Load AnimSystem [%s] failed", mName.c_str());
			return false;
		}

		return true;
	}

	// 资源卸载
	void AnimSystem::unloadImpl()
	{

	}

	// 设置骨架
	void AnimSystem::setSkeleton(const char* skeleton, AnimBlender* externalBlender)
	{
		if (m_skeleton)
		{
			if (m_skeleton->getName() != skeleton)
			{
				// 清空骨架
				if (m_skeleton)
					SkeletonManager::instance()->releaseResource(m_skeleton);

				// 清空动画
				cleanAnimations();

				// 设置骨架
				m_skeleton = SkeletonManager::instance()->createSkeleton(skeleton);
				m_skeleton->prepareLoad();
			}
		}
		else
		{
			// 清空动画
			cleanAnimations();

			// 设置骨架
			m_skeleton = SkeletonManager::instance()->createSkeleton(skeleton);
			m_skeleton->prepareLoad();
		}

		// 创建混合器
		if (m_blender)
			AnimManager::instance()->destroyAnimBlenders(&m_blender, 1);
		m_blender = AnimManager::instance()->createAnimBlender(m_skeleton);

		m_blenderExternal = externalBlender;

		// 匹配检测
		checkup();
	}

	// 添加动画
	void AnimSystem::addAnimation(const char* animPath)
	{
		if (!isAnimationExist(animPath))
		{
			Animation* animation = AnimManager::instance()->createAnim(animPath);
			if (animation)
			{
				if (!animation->prepareLoad())
				{
					AnimManager::instance()->releaseResource(animation);
					return;
				}

				m_animations.push_back(animation);
			}
		}
		else
		{
			EchoLogWarning("the animation [%s] is existed in animsystem [%s]", animPath, mName.c_str());
		}

		// 匹配检测
		checkup();
	}

	// 检测合法性
	bool AnimSystem::checkup()
	{
		if (m_skeleton)
		{
			for (size_t i = 0; i < m_animations.size(); i++)
			{
				if (m_skeleton->getBoneCount() != m_animations[i]->getBoneCount())
				{
					EchoLogError("skeleton [%s] can not used by animation [%s]", m_skeleton->getName().c_str(), m_animations[i]->getName().c_str());
					return false;
				}
			}
		}

		return true;
	}

	// 判断动画是否存在
	bool AnimSystem::isAnimationExist(const char* fileName)
	{
		for (size_t i = 0; i < m_animations.size(); i++)
		{
			if (m_animations[i]->getName() == fileName)
				return true;
		}

		return false;
	}

	// 清空动画
	void AnimSystem::cleanAnimations()
	{
		// 清空动画
		for (size_t i = 0; i < m_animations.size(); i++)
			AnimManager::instance()->releaseResource(m_animations[i]);

		m_animations.clear();
	}

	// 获取动画状态
	AnimState* AnimSystem::getAnimState(const char* animName)
	{
		// 是否已存在
		for (size_t i = 0; i < m_animStates.size(); i++)
		{
			if (m_animStates[i]->getAnim()->getName() == animName)
			{
				m_animStates[i]->reset();
				return m_animStates[i];
			}
		}

		// 新创建
		Animation* animation = dynamic_cast<Animation*>(AnimManager::instance()->getByName(animName));
		if (animation)
		{
			AnimState* animState = EchoNew(AnimState(animation));
			animState->reset();
			m_animStates.push_back(animState);

			return animState;
		}
		else
		{
			EchoLogError("The animation resource [%d] has not been loaded", animName);

			return NULL;
		}
	}

	AnimState* AnimSystem::getAnimStateNotCreate(const char* animName)
	{
		for (size_t i = 0; i < m_animStates.size(); i++)
		{
			if (m_animStates[i]->getAnim()->getName() == animName)
			{
				return m_animStates[i];
			}
		}

		return NULL;
	}

	// 保存
	void AnimSystem::save(const char* filePath)
	{
#ifdef ECHO_PLATFORM_WINDOWS
		xml_document<> doc;
		xml_node<>* xmlnode = doc.allocate_node(node_pi, doc.allocate_string("xml version='1.0' encoding='utf-8'"));
		xml_node<>* rootnode = doc.allocate_node(node_element, "animsys");

		doc.append_node(xmlnode);
		doc.append_node(rootnode);

		// 存储骨架名称
		if (m_skeleton)
		{
			xml_attribute<>* propertySkeleton = doc.allocate_attribute("skeleton", m_skeleton->getName().c_str());
			rootnode->append_attribute(propertySkeleton);
		}

		// 保存动画信息
		String animations;
		if (!m_animations.empty())
		{
			// 组织动画存储数据
			for (size_t i = 0; i < m_animations.size(); i++)
				animations += m_animations[i]->getName() + ";";

			// 添加到属性项
			xml_attribute<>* propertyanims = doc.allocate_attribute("anims", animations.c_str());
			rootnode->append_attribute(propertyanims);
		}

		std::ofstream out(filePath);
		out << doc;
		out.close();
#endif
	}

	__ImplementSingleton(AnimSystemManager);

	// 创建动画系统
	AnimSystem* AnimSystemManager::createAnimSystem(const String& name, bool isManual)
	{
		AnimSystem* animSystem = ECHO_DOWN_CAST<AnimSystem*>(ResourceManager::createResource(name, isManual)); EchoAssert(animSystem);

		return animSystem;
	}

	// 实现
	Resource* AnimSystemManager::createImpl(const String& name, bool isManual)
	{
		return EchoNew(AnimSystem(name, isManual));
	}
}