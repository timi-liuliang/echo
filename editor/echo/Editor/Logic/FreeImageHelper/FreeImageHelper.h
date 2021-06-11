#pragma once

#include <engine/core/base/type_def.h>
//#include <FreeImage/FreeImage.h>
#include <engine/core/util/StringUtil.h>
#include <engine/core/math/color.h>

namespace Echo {

    enum class ECOLOR_TYPE : uint8_t {
        ECT_MINISWHITE = 0,        // min value is white
        ECT_MINISBLACK = 1,        // min value is black
        ECT_RGB = 2,        // RGB color model
        ECT_PALETTE = 3,        // color map indexed
        ECT_RGBALPHA = 4,        // RGB color model with alpha channel
        ECT_CMYK = 5        // CMYK color model
    };

    class FreeImageHelper {
    public:
        struct ImageInfo {
            ui32 m_width;
            ui32 m_height;
            ECOLOR_TYPE m_colorType;
        };

    public:
        FreeImageHelper();

        ~FreeImageHelper();

        // instance
        static FreeImageHelper *instance();

        // get info
        bool getImageInfo(const char *filePath, ImageInfo &imageInfo);

        // scale image
        bool rescaleImage(const char *iFilePath, const char *oFilePath, float scaleValue = 1.f);

        bool rescaleImage(const char *iFilePath, const char *oFilePath, ui32 targetWidth, ui32 targetHeight);

        // save to
        bool saveImageToTGA(const char *iFilePath, const char *oFilePath, bool isResale = false, float scaleValue = 1.f);

        bool saveImageToBmp(const char *iFilePath, const char *oFilePath, bool isResale, ui32 width, ui32 height);

        int saveImageToBmp(Byte *pixelData, int widht, int height, const char *savePath);

        bool saveRGBAtoRGBjpeg(const char *srcFile, const char *dstFile, ImageInfo &imageInfo, float scale);

        // extract rgba channel
        bool extracRGBAlphaChannel(const char *srcPath, const char *oRgbFile, const char *oAlphaFile);


        // get image bits
        bool getImageBits(const char *filePath, unsigned char *bits);

        // add water mark
        bool addWaterMark(const char *dstFile, const char *srcFile);

        // merge image
        bool mergeIMage(StringArray *srcFile, const char *dstFile);

        // extract colors
        bool extractColors(const char *srcPath, std::vector<Echo::Color> &colors, int &width, int &height);
    };
}
