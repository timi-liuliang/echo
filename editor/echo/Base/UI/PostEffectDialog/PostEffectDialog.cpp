#include "PostEffectDialog.h"
#include "engine/core/Render/RenderStage/RenderStageManager.h"
#include "engine/core/Render/RenderTargetManager.h"
#include <engine/core/main/Root.h>
#include <QColorDialog>
#include <QtWidgets/QDoubleSpinBox>

namespace Studio
{
	PostEffectDialog::PostEffectDialog(QWidget* parent, Qt::WindowFlags f)
		: QDialog(parent, f)
		, m_ui(new Ui::PostEffectDialog)
		, m_renderStage(nullptr)
	{
		m_ui->setupUi(this);

		m_ui->brightnessLineEdit->setValidator(new QDoubleValidator(this));
		m_ui->sizeLineEdit->setValidator(new QDoubleValidator(this));

		auto manager = Echo::RenderStageManager::instance();
		if (!manager)
		{
			return;
		}
		m_renderStage = (Echo::PostProcessRenderStage*)manager->getRenderStageByID(Echo::RSI_PostProcess);
		if (!m_renderStage)
		{
			return;
		}

		bool enabled = m_renderStage->getImageEffectEnable("Bloom");
		m_ui->marmosetBloomEnableCheckBox->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);

		Echo::Vector4 v = m_renderStage->getImageEffectParameter("Bloom", "color");
		m_color = QColor(v.x * 255, v.y * 255, v.z * 255);
		m_ui->colorPushButton->setStyleSheet(QString("background-color: %1").arg(m_color.name()));
		float brightness = m_renderStage->getImageEffectParameter("Bloom", "brightness").x;
		m_ui->brightnessHSlider->setValue(sqrt(brightness * 1000.f));
		float bloomSize = m_renderStage->getImageEffectParameter("Bloom", "bloomSize").x;
		m_ui->sizeHSlider->setValue(bloomSize * 1000);

		enabled = EchoEngineSettings.isEnableToneMapping();
		m_ui->toneMappingEnableCheckBox->setChecked(enabled ? Qt::Checked : Qt::Unchecked);

		enabled = EchoEngineSettings.isEnableFXAA();
		m_ui->FXAAEnableCheckBox->setChecked(enabled ? Qt::Checked : Qt::Unchecked);

		enabled = m_renderStage->getImageEffectEnable("MotionBlur");
		auto blur = m_renderStage->getImageEffectParameter("MotionBlur", "blur_param").x;
		m_ui->motionBlurEnableCheckBox->setChecked(enabled ? Qt::Checked : Qt::Unchecked);
		m_ui->motionBlurDoubleSpinBox->setValue(blur);
	}

	PostEffectDialog::~PostEffectDialog()
	{
		delete m_ui;
	}

	void PostEffectDialog::on_marmosetBloomEnableCheckBox_stateChanged(int value)
	{
		EchoEngineSettings.setEnableBloom(value == Qt::Checked);
		if (m_renderStage)
		{
			m_renderStage->setImageEffectEnable("Bloom", value == Qt::Checked);
		}

		if (value == Qt::Checked)
		{
			m_ui->FXAAEnableCheckBox->setChecked(false);
		}
	}

	void PostEffectDialog::on_colorPushButton_clicked(bool)
	{
		if (!m_renderStage)
		{
			return;
		}
		auto color = QColorDialog::getColor(m_color, this, "Color", QColorDialog::DontUseNativeDialog);
		m_color = (color.spec() == QColor::Invalid) ? m_color : color;
		m_ui->colorPushButton->setStyleSheet(QString("background-color: %1").arg(m_color.name()));
		m_renderStage->setImageEffectParameter("Bloom", "color", Echo::Vector4(m_color.redF(), m_color.greenF(), m_color.blueF(), m_color.alphaF()));
	}

	void PostEffectDialog::on_brightnessHSlider_valueChanged(int value)
	{
		if (!m_renderStage)
		{
			return;
		}
		float v = value*value / 1000.f;
		m_ui->brightnessLineEdit->setText(QString("%1").arg(v, 0, 'f', 3));

		m_renderStage->setImageEffectParameter("Bloom", "brightness", Echo::Vector4(v, 0, 0, 0));
	}

	void PostEffectDialog::on_sizeHSlider_valueChanged(int value)
	{
		if (!m_renderStage)
		{
			return;
		}
		m_ui->sizeLineEdit->setText(QString("%1").arg(value/1000., 0, 'f', 3));
		m_renderStage->setImageEffectParameter("Bloom", "bloomSize", Echo::Vector4(value / 1000.f, 0, 0, 0));
	}

	void PostEffectDialog::on_brightnessLineEdit_returnPressed()
	{
		auto text = qobject_cast<QLineEdit*>(sender())->text();
		m_ui->brightnessHSlider->setValue(sqrt(text.toDouble() * 1000));
	}

	void PostEffectDialog::on_sizeLineEdit_returnPressed()
	{
		auto text = qobject_cast<QLineEdit*>(sender())->text();
		m_ui->sizeHSlider->setValue(text.toDouble() * 1000);
	}

	void PostEffectDialog::on_toneMappingEnableCheckBox_stateChanged(int value)
	{
		EchoEngineSettings.setEnableToneMapping(value == Qt::Checked);
	}

	void PostEffectDialog::on_FXAAEnableCheckBox_stateChanged(int value)
	{
		EchoEngineSettings.setEnableFXAA(value == Qt::Checked);
		if (m_renderStage)
		{
			m_renderStage->setImageEffectEnable("FXAA", value == Qt::Checked);
			if (value != Qt::Checked)
			{
				m_renderStage->setImageEffectSourceRT(Echo::RTI_LDRSceneColorMap);
			}
		}

		if (value == Qt::Checked)
		{
			m_ui->marmosetBloomEnableCheckBox->setChecked(false);
		}
	}

	void PostEffectDialog::on_motionBlurEnableCheckBox_stateChanged(int value)
	{
		if (m_renderStage)
		{
			m_renderStage->setImageEffectEnable("MotionBlur", value == Qt::Checked);
		}
	}

	void PostEffectDialog::on_motionBlurDoubleSpinBox_valueChanged(double value)
	{
		if (m_renderStage)
		{
			m_renderStage->setImageEffectParameter("MotionBlur", "blur_param", Echo::Vector4(value));
		}
	}
}  // namespace Studio
