#include "UIManager.h"
#include <Engine/Object/Root.h>

namespace Examples
{
	// 消息回调
	void HandleGoToFunction(const int &mapid, const int &xPos, const int &yPos, const int& zPos, const int& npcid, const int& chefu, int gotodirectly, const int64_t npckey, const int fubenid, const int taskid, const int messageid, const int tasklineid)
	{
		//LuaUtil::LargeInteger key = {};
		//key.quad = npckey;
		// 转至lua中处理
		//HeroGameApplication->FireTaskFindPathEvent(mapid, xPos, yPos, zPos, npcid, chefu, gotodirectly, key.low, key.high, fubenid, taskid, messageid, tasklineid);
	}

	// 显示提示
	void HandleShowRewardItemTips(const int& itemid, float xPos, float yPos, int equipid)
	{

	}

	void OnEmotionFrameChange(const CEGUI::EventArgs& e)
	{
// 		const CEGUI::RichEditBoxCoponentEventArgs& Args = static_cast<const CEGUI::RichEditBoxCoponentEventArgs&>(e);
// 		CEGUI::RichEditboxEmotionComponent* pEmtionCpn = static_cast<CEGUI::RichEditboxEmotionComponent*>(Args.d_Component);
// 
// 		if (pEmtionCpn != NULL)
// 		{
// 
// 			int aniID = pEmtionCpn->GetAniID();
// 			float fElapse = pEmtionCpn->GetElapseTime();
// 			int frame = GetEmotionManager()->GetFrameByElapseTime(aniID, fElapse);
// 			const CEGUI::Image* pImage = GetEmotionManager()->GetEmotionImageByFrame(aniID, frame);
// 			if (pImage != NULL)
// 			{
// 				pEmtionCpn->setImage(pImage);
// 			}
// 
// 		}

		return;
	}

	// 点击Tips链接
	void HandleTipsLinkClick(const CEGUI::CEGUIString& name, int64_t roleID, int type, int key, int baseid, int64_t shopID, int counterID, const CEGUI::colour& nameColor, int bind, int64_t loseeffecttime)
	{

	}

	//...
	void HandleTeamRecruitClick(int64_t teamid, int recruittype, int param1, int param2)
	{

	}

	void HandleCharNameClick(int64_t roleID, const CEGUI::CEGUIString& roleName, int level, int64_t familyid)//,int recoredID) 不知道为什么回调参数对应不上，多了一个recoreid，先去掉，有问题再加回来
	{

	}

	void HandleChangelImageClick(const int& changelID)
	{

	}

	void HandleShowCompnentTips(const CEGUI::RichEditboxComponent* pCompnent)
	{

	}

	LORD::Dword HandleUIPlaySound(const CEGUI::CEGUIString& soundRes)
	{
		return 0;
	}

	void HandleLinkTextClick(int keyid)
	{

	}

	// 点击可编辑区域
	void ClickEditArea(const char* str)
	{
	}

	// 什么鬼？
	void RestoreXPRenderState(void)
	{

	}

	// 这么像逻辑呢...
	bool HandleCheckShied(CEGUI::CEGUIString& inText)
	{
		return false;
	}

	// 这又什么？
	void GetEmotionParseInf(int id, bool& bExist, bool& bExitstNextChar)
	{

	}

	// 构造函数
	UIManager::UIManager()
		: m_rootWindow( NULL)
	{
	}

	// 析构函数
	UIManager::~UIManager()
	{
		CEGUI::CEGUIRenderer::destroySystem();
	}

	// 初始化
	void UIManager::initialize()
	{
		m_uiRenderer = &CEGUI::CEGUIRenderer::bootstrapSystem();

		// 设置适配器
		LORD::Viewport* viewPort = LordRender->getFrameBuffer()->getViewport();
		m_adapter.Initialize(viewPort->getWidth(), viewPort->getHeight());
		CEGUI::System::getSingletonPtr()->SetAdapter(&m_adapter);

		// 设置显示器尺寸
		m_uiRenderer->setDisplaySize(CEGUI::Size( (float)m_adapter.GetLogicWidth(), (float)m_adapter.GetLogicHeight()));
		m_uiResourceProvider = CEGUI::System::getSingleton().getResourceProvider<CEGUI::ResourceProvider>();

		// 关联引擎渲染调用
		LORD::RenderQueue* pUIRenderQuene = LordSceneManager->getRenderQueue("UI");
		pUIRenderQuene->setManalRenderEnd((LORD::ManualRenderListener*)m_uiRenderer);

		// 设置日志级别
		CEGUI::LoggingLevel loggingMap[] = {
			CEGUI::Insane,
			CEGUI::Informative,
			CEGUI::Warnings,
			CEGUI::Errors,
			CEGUI::Errors,
		};
		CEGUI::Logger::getSingleton().setLoggingLevel(loggingMap[LordLogManager->getLogLevel()]);

		// 初始化资源组
		initResourceDirectoriesAndGroups();

		// 加载Scheme
		CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
		CEGUI::SchemeManager::getSingleton().create("TaharezLook2.scheme");

		// 加载UI动画
		CEGUI::AnimationManager::getSingleton().loadAnimationsFromXML("example.xml", "animations");

		// 设置鼠标图片等
		CEGUI::System::getSingleton().setDefaultMouseCursor("component", "Mouse");
		CEGUI::System::getSingleton().setDefaultTooltip("TaharezLook/Tooltip");
		CEGUI::System::getSingleton().setDefaultCompnenttip("TaharezLook/CompnentTip");

		// 设置下划线纹理，刷子所用纹理，子体刷纹理，默认字体等
		CEGUI::Image* pUnderLineImage = (CEGUI::Image*)&CEGUI::ImagesetManager::getSingleton().get("component").getImage("UnderLine");
		CEGUI::Image* pTextBrushImage = (CEGUI::Image*)&CEGUI::ImagesetManager::getSingleton().get("component").getImage("TextBrush");
		CEGUI::FontManager::getSingleton().SetUnderLineImage(pUnderLineImage);
		CEGUI::System::getSingleton().SetTextBrushImage(pTextBrushImage);
		CEGUI::System::getSingleton().setDefaultFont("simhei-12");

		// 回调绑定
		CEGUI::System::getSingleton().SetDefaultGoToFunction((CEGUI::GoToFunction*)(&HandleGoToFunction));
		CEGUI::System::getSingleton().SetDefaultShowItemTips((CEGUI::ShowItemTips*)(&HandleShowRewardItemTips));
		CEGUI::System::getSingleton().SetEmotionFrameChangeFunction((CEGUI::EmotionChangeFrameFunction*)(&OnEmotionFrameChange));
		CEGUI::System::getSingleton().SetTipsLinkClickFunc((CEGUI::TipsLinkClick*)(&HandleTipsLinkClick));
		CEGUI::System::getSingleton().SetDefaultTeamRecruitFunction((CEGUI::OnTeamRecruitClick*)(&HandleTeamRecruitClick));
		CEGUI::System::getSingleton().SetNameClickFuction((CEGUI::OnNameLinkClick*)(&HandleCharNameClick));
		CEGUI::System::getSingleton().SetChangelClickFuction((CEGUI::OnChangelImageClick*)(&HandleChangelImageClick));
		CEGUI::System::getSingleton().SetCompnentTips((CEGUI::ShowCompnentTips*)(&HandleShowCompnentTips));
		CEGUI::System::getSingleton().SetUIPlaySoundFunc((CEGUI::OnUIPlaySound*)(&HandleUIPlaySound));
		CEGUI::System::getSingleton().SetLinkTextClickFuction((CEGUI::OnLinkTextClick*)(&HandleLinkTextClick));
		CEGUI::System::getSingleton().SetCheckShiedFunc((CEGUI::OnCheckShied*)(&HandleCheckShied));
		CEGUI::System::getSingleton().SetRestoreXPStateFunc((CEGUI::OnRestoreXPRenderState*)(&RestoreXPRenderState));
		CEGUI::System::getSingleton().SetOnClickEditAreaFunc((CEGUI::OnClickEditArea*)(&ClickEditArea));
		
		CEGUI::System::getSingleton().SetEmotionParseInfFunc((CEGUI::EmotionParseInfFunc*)(&GetEmotionParseInf));
		CEGUI::System::getSingleton().SetEmotionNum(49);

		// 创建主窗口
		m_rootWindow = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "root_wnd");
		m_rootWindow->setMousePassThroughEnabled(true);

		// 设置主窗口
		CEGUI::System::getSingleton().setGUISheet(m_rootWindow);
		m_rootWindow->setMousePassThroughEnabled(true);
		m_rootWindow->setDistributesCapturedInputs(true);

		// 模态
		CEGUI::System::getSingleton().subscribeEvent(CEGUI::System::EventStartModalState, CEGUI::Event::Subscriber(&UIManager::onStartModalState, this));

		// 加载layout前调用
		CEGUI::WindowManager::getSingleton().subscribeEvent(CEGUI::WindowManager::EventBeforeLoadlayout,CEGUI::Event::Subscriber(&UIManager::handleBeforeLoadlayout, this));

		// 初始化Lua脚本模块
		//CEGUI::System::getSingletonPtr()->setScriptingModule( &CEGUI::LuaScriptModule::create(HeroCCLuaEngine->getLuaState()));
	}

	// 初始化资源目录
	void UIManager::initResourceDirectoriesAndGroups()
	{
// 		std::string pathPrefix = "/";
// 		m_uiResourceProvider->setResourceGroupDirectory("animations", pathPrefix + "ui/animations");
// 		m_uiResourceProvider->setResourceGroupDirectory("fonts", pathPrefix + "ui/fonts");
// 		m_uiResourceProvider->setResourceGroupDirectory("imagesets", pathPrefix + "ui/imagesets");
// 		m_uiResourceProvider->setResourceGroupDirectory("layouts", pathPrefix + "ui/layouts");
// 		m_uiResourceProvider->setResourceGroupDirectory("looknfeels", pathPrefix + "ui/looknfeel");
// 		m_uiResourceProvider->setResourceGroupDirectory("lua_scripts", pathPrefix + "ui/lua_scripts");
// 		m_uiResourceProvider->setResourceGroupDirectory("schemes", pathPrefix + "ui/schemes");
// 		m_uiResourceProvider->setResourceGroupDirectory("schemas", pathPrefix + "ui/xml_schemas");

		CEGUI::Imageset::setDefaultResourceGroup("imagesets");
		CEGUI::Font::setDefaultResourceGroup("fonts");
		CEGUI::Scheme::setDefaultResourceGroup("schemes");
		CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
		CEGUI::WindowManager::setDefaultResourceGroup("layouts");
		CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
		CEGUI::AnimationManager::setDefaultResourceGroup("animations");
	}

	// 窗口大小改变
	void UIManager::onSize(int screenW, int screenH)
	{
		UIAdapter* adapter = LORD_DOWN_CAST<UIAdapter*>(CEGUI::System::getSingleton().GetAdapter());
		if (adapter)
		{
			adapter->Initialize(screenW, screenH);
		}
	}

	// 更新
	void UIManager::tick(int delta)
	{
		CEGUI::System::getSingleton().injectTimePulse(delta * 0.001f);
	}

	// 事件
	bool UIManager::onStartModalState(const CEGUI::EventArgs& e)
	{
		return true;
	}

	// 事件
	bool UIManager::handleBeforeLoadlayout(const CEGUI::EventArgs &e)
	{
		return true;
	}
}