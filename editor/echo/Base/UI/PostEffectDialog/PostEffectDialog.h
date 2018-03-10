#pragma once

#include "ui_PostEffectDialog.h"
namespace Echo
{
	class PostProcessRenderStage;
} // namespace LROD

namespace Studio
{
	class PostEffectDialog : public QDialog
	{
		Q_OBJECT
	public:
		explicit PostEffectDialog(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
		~PostEffectDialog();

	private slots:
		void on_marmosetBloomEnableCheckBox_stateChanged(int);
		void on_colorPushButton_clicked(bool);
		void on_brightnessHSlider_valueChanged(int);
		void on_sizeHSlider_valueChanged(int);
		void on_brightnessLineEdit_returnPressed();
		void on_sizeLineEdit_returnPressed();
		void on_toneMappingEnableCheckBox_stateChanged(int);
		void on_FXAAEnableCheckBox_stateChanged(int);
		void on_motionBlurEnableCheckBox_stateChanged(int);
		void on_motionBlurDoubleSpinBox_valueChanged(double);

	private:
		Ui::PostEffectDialog* m_ui;
		QColor m_color;
		Echo::PostProcessRenderStage* m_renderStage;
	};
}  // namespace Studio
