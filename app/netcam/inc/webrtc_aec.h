#ifndef _GK_AEC_H__
#define _GK_AEC_H__


#ifdef __cplusplus
extern "C" {
#endif


int webrtc_aec_register(int *ptrAecHandle);
int webrtc_aec_unregister(int aecHandle);



#ifdef __cplusplus
}
#endif

#endif /* _GK_AEC_H__ */
