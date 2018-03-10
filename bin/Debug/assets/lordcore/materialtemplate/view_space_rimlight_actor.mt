<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="view_space_rimlight_actor.vs" ps="view_space_rimlight_actor.ps">
		<BlendState>
			<ColorWriteMask value="CMASK_ALL"/>
		</BlendState>
		<RasterizerState>
		</RasterizerState>
		<DepthStencilState>
		</DepthStencilState>
		<SamplerState>
			<BiLinearMirror>
				<MinFilter value="FO_LINEAR"/>
				<MagFilter value="FO_LINEAR"/>
				<MipFilter value="FO_NONE"/>
				<AddrUMode value="AM_MIRROR"/>
				<AddrVMode value="AM_MIRROR"/>
			</BiLinearMirror>
		</SamplerState>
		<Texture>
			<stage no="0" sampler = "BiLinearMirror"/>
			<stage no="1" sampler = "BiLinearMirror"/>
			<stage no="2" sampler = "BiLinearMirror"/>
			<stage no="3" sampler = "BiLinearMirror"/>
			<stage no="4" sampler = "BiLinearMirror"/>
		</Texture>
		<Macros>
			<Macro value="ALPHA_TEST" default="false"/>
			<Macro value="SPRITE_ANIMATION" default="false"/>
			<Macro value="RIME_LIGHT" default="true"/>
		</Macros>
		<DefaultUniformValue>
			<Unifrom name="uFadeScale" type="SPT_FLOAT" count="1" value="0" />
		</DefaultUniformValue>
	</ShaderPrograme>
</MaterialDesc>
