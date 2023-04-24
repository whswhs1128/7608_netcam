/**
 * goolink_adjustinterface.h
 *
 * External APIs for device side implementation.
 *
 *
 */

#ifndef __GOO_LINK_ADJUSTINTERFACE_H__
#define __GOO_LINK_ADJUSTINTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif



// Return:
// >0  :bites sent to cache;
// <0 : error, reference to goolik_apiv2.h
int glnk_sendvideodataExtData( unsigned char channel,
								unsigned char ismainorsub,
			                    char isIFrame, 
			                    void* videoData, 
			                    unsigned int videoDataLen,
			                    void* extData,
			                    unsigned int extDataLen);

// Return:
// >0  :bites sent to cache;
// <0 : error, reference to goolik_apiv2.h
int glnk_sendaudiodataExtData(	unsigned char channel, 
						void* audioData, 
						uint32_t audioDataLen,
						void* extData,
			            unsigned int extDataLen);


int32_t glnk_sendplaybackdataExData(	unsigned char channel,
							char isIFrame,
							char AudioOrVideo,
						void* Data,
						uint32_t StreamTime,
						uint32_t DataLen,
						void* extData,
			             unsigned int extDataLen);

#ifdef __cplusplus
}
#endif

#endif // __GOO_LINK_API_H__

