#ifndef FACE_6323_H
#define FACE_6323_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int xPos;      
    int yPos;      
    int width;    
    int height; 
}fb_rect_t;



int goke_upgrade_start(void);
int goke_upgrade_fresh_now(void);

#ifdef __cplusplus
    }
#endif

#endif

