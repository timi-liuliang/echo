/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Proprietary Software license
 * that can be found at http://live2d.com/eula/live2d-proprietary-software-license-agreement_en.html.
 */


#ifndef LIVE2D_CUBISM_CORE_H
#define LIVE2D_CUBISM_CORE_H


#if defined (__cplusplus)
extern "C"
{
#endif

    /* ------- *
     * DEFINES *
     * ------- */

    /** Core API attribute. */ 
    #if !defined (csmApi)
    #define csmApi 
    #endif


    /* ----- *
     * TYPES *
     * ----- */

    /** Cubism moc. */
    typedef struct csmMoc csmMoc;


    /** Cubism model. */
    typedef struct csmModel csmModel;


    /** Cubism version identifier. */
    typedef unsigned int csmVersion;


    /** Alignment constraints. */
    enum
    {
    /** Necessary alignment for mocs (in bytes). */
    csmAlignofMoc = 64,
        
    /** Necessary alignment for models (in bytes). */
    csmAlignofModel = 16
    };


    /** Bit masks for non-dynamic drawable flags. */
    enum
    {
    /** Additive blend mode mask. */
    csmBlendAdditive = 1 << 0,

    /** Multiplicative blend mode mask. */
    csmBlendMultiplicative = 1 << 1,

    /** Double-sidedness mask. */
    csmIsDoubleSided = 1 << 2
    };


    /** Bit masks for dynamic drawable flags. */
    enum
    {
        /** Flag set when visible. */
        csmIsVisible = 1 << 0,
        /** Flag set when visibility did change. */
        csmVisibilityDidChange = 1 << 1,
        /** Flag set when opacity did change. */
        csmOpacityDidChange = 1 << 2,
        /** Flag set when draw order did change. */
        csmDrawOrderDidChange = 1 << 3,
        /** Flag set when render order did change. */
        csmRenderOrderDidChange = 1 << 4,
        /** Flag set when vertex positions did change. */
        csmVertexPositionsDidChange = 1 << 5
    };


    /** Bitfield. */
    typedef unsigned char csmFlags;


    /** 2 component vector. */
    typedef struct
    {
        /** First component. */
        float X;

        /** Second component. */
        float Y;
    }
    csmVector2;


    /** Log handler.
     *
     * @param  message  Null-terminated string message to log.
     */
    typedef void (*csmLogFunction)(const char* message);


    /* ------- *
     * VERSION *
     * ------- */

    /**
     * Queries Core version.
     *
     * @return  Core version.
     */
    csmApi csmVersion csmGetVersion();


    /* ------- *
    * LOGGING *
    * ------- */

    /**
     * Queries log handler.
     *
     * @return  Log handler.
     */
    csmApi csmLogFunction csmGetLogFunction();


    /**
     * Sets log handler.
     *
     * @param  handler  Handler to use.
     */
    csmApi void csmSetLogFunction(csmLogFunction handler);


    /* --- *
     * MOC *
     * --- */

    /**
     * Tries to revive a moc from bytes in place.
     *
     * @param  address  Address of unrevived moc. The address must be aligned to 'csmAlignofMoc'.
     * @param  size     Size of moc (in bytes).
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi csmMoc* csmReviveMocInPlace(void* address, const unsigned int size);


    /* ----- *
     * MODEL *
     * ----- */

    /**
     * Queries size of a model in bytes.
     *
     * @param  moc  Moc to query.
     *
     * @return  Valid size on success; '0' otherwise.
     */
    csmApi unsigned int csmGetSizeofModel(const csmMoc* moc);


    /**
     * Tries to instantiate a model in place.
     *
     * @param  moc      Source moc.
     * @param  address  Address to place instance at. Address must be aligned to 'csmAlignofModel'.
     * @param  size     Size of memory block for instance (in bytes).
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi csmModel* csmInitializeModelInPlace(const csmMoc* moc,
                                                void* address,
                                                const unsigned int size);

    /**
     * Updates a model.
     *
     * @param  model  Model to update.
     */
    csmApi void csmUpdateModel(csmModel* model);


    /* ------ *
     * CANVAS *
     * ------ */

    /**
     * Reads info on a model canvas.
     *
     * @param  model              Model query.
     *
     * @param  outSizeInPixels    Canvas dimensions.
     * @param  outOriginInPixels  Origin of model on canvas.
     * @param  outPixelsPerUnit   Aspect used for scaling pixels to units.
     */
    csmApi void csmReadCanvasInfo(const csmModel* model,
                                  csmVector2* outSizeInPixels,
                                  csmVector2* outOriginInPixels,
                                  float* outPixelsPerUnit);


    /* ---------- *
     * PARAMETERS *
     * ---------- */

    /**
     * Gets number of parameters.
     *
     * @param[in]  model  Model to query.
     *
     * @return  Valid count on success; '-1' otherwise.
     */
    csmApi int csmGetParameterCount(const csmModel* model);

    /**
     * Gets parameter IDs.
     * All IDs are null-terminated ANSI strings.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const char** csmGetParameterIds(const csmModel* model);

    /**
     * Gets minimum parameter values.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const float* csmGetParameterMinimumValues(const csmModel* model);

    /**
     * Gets maximum parameter values.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const float* csmGetParameterMaximumValues(const csmModel* model);

    /**
     * Gets default parameter values.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const float* csmGetParameterDefaultValues(const csmModel* model);

    /**
     * Gets read/write parameter values buffer.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi float* csmGetParameterValues(csmModel* model);


    /* ----- *
     * PARTS *
     * ----- */

    /**
     * Gets number of parts.
     *
     * @param  model  Model to query.
     *
     * @return  Valid count on success; '-1' otherwise.
     */
    csmApi int csmGetPartCount(const csmModel* model);

    /**
     * Gets parts IDs.
     * All IDs are null-terminated ANSI strings.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const char** csmGetPartIds(const csmModel* model);

    /**
     * Gets read/write part opacities buffer.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi float* csmGetPartOpacities(csmModel* model);


    /* --------- *
     * DRAWABLES *
     * --------- */

    /**
     * Gets number of drawables.
     *
     * @param  model  Model to query.
     *
     * @return  Valid count on success; '-1' otherwise.
     */
    csmApi int csmGetDrawableCount(const csmModel* model);

    /**
     * Gets drawable IDs.
     * All IDs are null-terminated ANSI strings.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const char** csmGetDrawableIds(const csmModel* model);

    /**
     * Gets constant drawable flags.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const csmFlags* csmGetDrawableConstantFlags(const csmModel* model);

    /**
     * Gets dynamic drawable flags.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const csmFlags* csmGetDrawableDynamicFlags(const csmModel* model);

    /**
     * Gets drawable texture indices.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const int* csmGetDrawableTextureIndices(const csmModel* model);

    /**
     * Gets drawable draw orders.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const int* csmGetDrawableDrawOrders(const csmModel* model);

    /**
     * Gets drawable render orders.
     * The higher the order, the more up front a drawable is.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0'otherwise.
     */
    csmApi const int* csmGetDrawableRenderOrders(const csmModel* model);

    /**
     * Gets drawable opacities.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const float* csmGetDrawableOpacities(const csmModel* model);

    /**
     * Gets numbers of masks of each drawable.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const int* csmGetDrawableMaskCounts(const csmModel* model);

    /**
     * Gets mask indices of each drawable.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const int** csmGetDrawableMasks(const csmModel* model);

    /**
     * Gets number of vertices of each drawable.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const int* csmGetDrawableVertexCounts(const csmModel* model);

    /**
     * Gets vertex position data of each drawable.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; a null pointer otherwise.
     */
    csmApi const csmVector2** csmGetDrawableVertexPositions(const csmModel* model);

    /**
     * Gets texture coordinate data of each drawables.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const csmVector2** csmGetDrawableVertexUvs(const csmModel* model);

    /**
     * Gets number of triangle indices for each drawable.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const int* csmGetDrawableIndexCounts(const csmModel* model);

    /**
     * Gets triangle index data for each drawable.
     *
     * @param  model  Model to query.
     *
     * @return  Valid pointer on success; '0' otherwise.
     */
    csmApi const unsigned short** csmGetDrawableIndices(const csmModel* model);

    /**
     * Resets all dynamic drawable flags.
     *
     * @param  model  Model containing flags.
     */
    csmApi void csmResetDrawableDynamicFlags(csmModel* model);


#if defined (__cplusplus)
}
#endif

#endif
