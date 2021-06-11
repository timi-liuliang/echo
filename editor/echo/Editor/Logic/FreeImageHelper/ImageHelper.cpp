#include "ImageHelper.h"
#include "engine/core/util/PathUtil.h"
#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"
#include "stb/stb_image_write.h"

namespace Echo
{
    static ImageHelper g_freeImageHelper;

    ImageHelper::ImageHelper() 
    {
    }

    ImageHelper::~ImageHelper() 
    {
    }

    ImageHelper *ImageHelper::instance()
    {
        static ImageHelper *inst = EchoNew(ImageHelper);
        return inst;
    }

    bool ImageHelper::getImageInfo(const char *filePath, ImageInfo &imageInfo)
    {
        int x = 0;
        int y = 0;
        int channels = 0;
        if (!stbi_info(filePath, &x, &y, &channels)) {
            return false;
        }
        imageInfo.m_width = x;
        imageInfo.m_height = y;
        imageInfo.m_colorType = channels == 3 ? ECOLOR_TYPE::ECT_RGB : ECOLOR_TYPE::ECT_RGBALPHA;
        return true;
    }


    bool ImageHelper::rescaleImage(const char *iFilePath, const char *oFilePath, float scaleValue) 
    {
        int x = 0;
        int y = 0;
        int channels = 0;

        if (!stbi_info(iFilePath, &x, &y, &channels)) {
            return false;
        }

        int nx = x * scaleValue;
        int ny = y * scaleValue;
        return rescaleImage(iFilePath, oFilePath, nx, ny);
    }

    bool ImageHelper::rescaleImage(const char *iFilePath, const char *oFilePath, ui32 targetWidth, ui32 targetHeight)
    {
        int x = 0;
        int y = 0;
        int channels_in_file = 0;
        stbi_uc *pixels = stbi_load(iFilePath, &x, &y, &channels_in_file, 0);
        if (!pixels) {
            return false;
        }

        int nx = targetWidth;
        int ny = targetHeight;
        int stride = nx * channels_in_file;
        stbi_uc *output_pixels = (stbi_uc *) malloc(nx * ny * channels_in_file);

        if (!stbir_resize_uint8(pixels, x, y, x * channels_in_file, output_pixels, nx, ny, stride, channels_in_file)) {
            free(pixels);
            free(output_pixels);
            return false;
        }

        std::string ext = PathUtil::GetFileExt(oFilePath).c_str();
        int result = 0;

        // tolower???
        if (ext == "png") {
            result = stbi_write_png(oFilePath, nx, ny, channels_in_file, output_pixels, stride);
        } else if (ext == "bmp") {
            result = stbi_write_bmp(oFilePath, nx, ny, channels_in_file, output_pixels);
        } else if (ext == "tga") {
            result = stbi_write_tga(oFilePath, nx, ny, channels_in_file, output_pixels);
        } else if (ext == "jpg") {
            result = stbi_write_jpg(oFilePath, nx, ny, channels_in_file, output_pixels, 100);
        }

        free(pixels);
        free(output_pixels);
        return result;
    }

    stbi_uc *scaleImageImpl(const char *iFilePath, int tw, int th) 
    {
        int x = 0;
        int y = 0;
        int channels_in_file = 0;
        stbi_uc *pixels = stbi_load(iFilePath, &x, &y, &channels_in_file, 0);
        if (!pixels) 
        {
            return nullptr;
        }

        if (tw == x && th == y) 
        {
            return pixels;
        }

        int nx = tw;
        int ny = th;
        int stride = nx * channels_in_file;
        stbi_uc *output_pixels = (stbi_uc *) malloc(nx * ny * channels_in_file);

        if (!stbir_resize_uint8(pixels, x, y, x * channels_in_file, output_pixels, nx, ny, stride, channels_in_file))
        {
            free(pixels);
            free(output_pixels);
            return nullptr;
        }

        free(pixels);
        return output_pixels;
    }

    stbi_uc *scaleImageImpl(const char *iFilePath, float scale, int &nx, int &ny, int &nc) 
    {
        int x = 0;
        int y = 0;
        int channels = 0;
        if (!stbi_info(iFilePath, &x, &y, &channels)) {
            return nullptr;
        }

        nx = scale * x;
        ny = scale * y;
        nc = channels;
        return scaleImageImpl(iFilePath, nx, ny);
    }

    bool ImageHelper::saveImageToTGA(const char *iFilePath, const char *oFilePath, bool isResale, float scaleValue) 
    {
        int nx = 0;
        int ny = 0;
        int nc = 0;
        stbi_uc *output_pixels = scaleImageImpl(iFilePath, isResale ? scaleValue : 1, nx, ny, nc);
        if (!output_pixels) {
            return false;
        }

        int result = stbi_write_tga(oFilePath, nx, ny, nc, output_pixels);
        return result;
    }

    bool ImageHelper::saveImageToBmp(const char *iFilePath, const char *oFilePath, bool isResale, ui32 width, ui32 height) 
    {
        int x = 0;
        int y = 0;
        int channels = 0;
        if (!stbi_info(iFilePath, &x, &y, &channels)) {
            return false;
        }


        int nx = isResale ? width : x;
        int ny = isResale ? height : y;
        int nc = channels;
        stbi_uc *output_pixels = scaleImageImpl(iFilePath, nx, ny);
        if (!output_pixels) {
            return false;
        }

        int result = stbi_write_bmp(oFilePath, nx, ny, nc, output_pixels);
        return result;
    }

    bool ImageHelper::extracRGBAlphaChannel(const char *srcPath, const char *oRgbFile, const char *oAlphaFile) 
    {
        int x = 0;
        int y = 0;
        int channels_in_file = 0;
        stbi_uc *pixels = stbi_load(srcPath, &x, &y, &channels_in_file, 0);
        if (!pixels) {
            return false;
        }

        if (channels_in_file != 4) {
            free(pixels);
            return false;
        }

        stbi_uc *rgb = (stbi_uc *) malloc(x * y * 3);
        stbi_uc *alpha = (stbi_uc *) malloc(x * y * 3);

        for (int h = 0; h < y; ++h) {
            for (int w = 0; w < x; ++w) {
                rgb[h * x * 3 + w * 3 + 0] = pixels[h * x * 4 + w * 4 + 0];
                rgb[h * x * 3 + w * 3 + 1] = pixels[h * x * 4 + w * 4 + 1];
                rgb[h * x * 3 + w * 3 + 2] = pixels[h * x * 4 + w * 4 + 2];

                alpha[h * x * 3 + w * 3 + 0] = pixels[h * x * 4 + w * 4 + 3];
                alpha[h * x * 3 + w * 3 + 1] = pixels[h * x * 4 + w * 4 + 3];
                alpha[h * x * 3 + w * 3 + 2] = pixels[h * x * 4 + w * 4 + 3];
            }
        }

        int result = true;
        result = result && stbi_write_bmp(oRgbFile, x, y, 3, rgb);
        result = result && stbi_write_bmp(oAlphaFile, x, y, 3, alpha);
        return result;
    }

    bool ImageHelper::extractColors(const char *srcPath, std::vector<Echo::Color> &colors, int &width, int &height) 
    {
        int x = 0;
        int y = 0;
        int channels_in_file = 0;
        stbi_uc *pixels = stbi_load(srcPath, &x, &y, &channels_in_file, 0);
        if (!pixels) {
            return false;
        }

        if (channels_in_file != 4) {
            free(pixels);
            return false;
        }

        colors.resize(x * y);
        colors.clear();
        for (int h = 0; h < y; ++h) {
            for (int w = 0; w < x; ++w) {
                int offset = h * x * 4 + w * 4;
                Byte r = pixels[offset + 0];
                Byte g = pixels[offset + 1];
                Byte b = pixels[offset + 2];
                Byte a = pixels[offset + 3];

                colors.emplace_back(Echo::Color::fromRGBA(r, g, b, a));
            }
        }

        return true;
    }

#define GCC_PACK(n)

    int ImageHelper::saveImageToBmp(Byte *pixelData, int width, int height, const char *savePath) 
    {
        PathUtil::DelPath(savePath);
        stbi_write_bmp(savePath, width, height, 3, pixelData);
        return true;
    }

    bool ImageHelper::getImageBits(const char *filePath, unsigned char *bits) 
    {
        int x = 0;
        int y = 0;
        int channels_in_file = 0;
        stbi_uc *pixels = stbi_load(filePath, &x, &y, &channels_in_file, 0);
        if (!pixels) {
            return false;
        }
        memcpy(bits, pixels, x * y * channels_in_file);

        free(pixels);
        return true;
    }

    bool ImageHelper::addWaterMark(const char *dstFile, const char *srcFile) 
    {
        int dx = 0;
        int dy = 0;
        int dc = 0;
        stbi_uc *dp = stbi_load(dstFile, &dx, &dy, &dc, 0);
        if (!dp) 
        {
            return false;
        }

        int sx = 0;
        int sy = 0;
        int sc = 0;
        stbi_uc *sp = stbi_load(dstFile, &sx, &sy, &sc, 0);
        if (!sp) 
        {
            free(dp);
            return false;
        }

        if ((dc - 3) * (dc - 4) * (sc - 3) * (sc - 4))
            return false;

        for (int y = 0; y < dy && y < sy; ++y) 
        {
            for (int x = 0; x < dx && x < sx; ++x) 
            {
                int doffset = (y * dx + x) * dc;
                int soffset = (y * sx + x) * sc;

                ui32 dr = dp[doffset + 0];
                ui32 dg = dp[doffset + 1];
                ui32 db = dp[doffset + 2];
                ui32 da = dc == 3 ? 1 : dp[doffset + 3];

                ui32 sr = sp[soffset + 0];
                ui32 sg = sp[soffset + 1];
                ui32 sb = sp[soffset + 2];
                ui32 sa = sc == 3 ? 1 : sp[soffset + 3];

                Byte a = (Byte) (255 - (255 - da) * (255 - sa) / 255);
                Byte r = (Byte) ((dr * da * (255 - sa) + sr * sa) / a);
                Byte g = (Byte) ((dg * da * (255 - sa) + sg * sa) / a);
                Byte b = (Byte) ((db * da * (255 - sa) + sb * sa) / a);

                dp[doffset + 0] = r;
                dp[doffset + 1] = g;
                dp[doffset + 2] = b;
                if (dc == 4) {
                    dp[doffset + 3] = a;
                }
            }
        }

        int result = stbi_write_tga(dstFile, dx, dy, dc, dp);
        free(dp);
        free(sp);
        return result;
    }

    bool ImageHelper::saveRGBAtoRGBjpeg(const char *srcFile, const char *dstFile, ImageInfo &imageInfo, float scale) 
    {
        int sx = 0;
        int sy = 0;
        int sc = 0;
        stbi_uc *sp = stbi_load(dstFile, &sx, &sy, &sc, 0);
        if (!sp) 
            return false;

        if (sc != 4) 
        {
            free(sp);
            return false;
        }

        stbi_uc *dp = (stbi_uc *) malloc(sx * sy * 3 * 2);
        int offseto = sx * sy * 3;
        for (size_t pix = 0; pix < sx * sy; pix++) 
        {
            int offset = pix * 3;
            dp[offset + 0] = sp[pix * 4 + 0];
            dp[offset + 1] = sp[pix * 4 + 1];
            dp[offset + 2] = sp[pix * 4 + 2];
            dp[offseto + offset + 0] = sp[pix * 4 + 3];
            dp[offseto + offset + 1] = sp[pix * 4 + 3];
            dp[offseto + offset + 2] = sp[pix * 4 + 3];
        }

        int result = stbi_write_tga(dstFile, sx, sy, 3, dp);
        free(dp);
        free(sp);
        return result;
    }

    bool ImageHelper::mergeIMage(StringArray *srcFile, const char *dstFile) 
    {
        if ((*srcFile).empty())
            return false;

        return false;
    }
}
