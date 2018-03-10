<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="glow.vs" ps="glow.ps">
		<BlendState>
		</BlendState>
		<RasterizerState>
		</RasterizerState>
		<DepthStencilState>
			<DepthEnable value="true"/>
			<WriteDepth value="false"/>
			<DepthFunc value="CF_LESS_EQUAL"/>
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
			<Macro value="GLOW" default="true" />
			<Macro value="RECEIVE_FOG" default="true" />
			<Macro value="GROW" default="false" />
		</Macros>
		<DefaultUniformValue>
			<Unifrom name="DiffuseIntensity" type="SPT_FLOAT" count="1" value="1.0" />
		</DefaultUniformValue>	
	</ShaderPrograme>
</MaterialDesc>
