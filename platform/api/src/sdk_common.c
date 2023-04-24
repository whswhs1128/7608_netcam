#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "errno.h"

#include "sdk_def.h"
#include "sdk_debug.h"

extern int new_system_call(const char *cmdline);

int sdk_venc_bps_limit(int const enc_width, int const enc_height, int const enc_fps, int const enc_bps)
{
	int const enc_size = enc_width * enc_height;
	int max_bps = 0;

	if(enc_size <= (180 * 144)){
		max_bps = 256; // kbps
	}else if(enc_size <= (360 * 144)){
		max_bps = 384;
	}else if(enc_size <= (480 * 144)){
		max_bps = 480;
	}else if(enc_size <= (360 * 288)){
		max_bps = 512; // kbps
	}else if(enc_size <= (480 * 288)){
		max_bps = 640;
	}else if(enc_size <= (720 * 288)){
		max_bps = 768;
	}else if(enc_size <= (960 * 288)){
		max_bps = 1024;
	}else if(enc_size <= (720 * 576)){
		max_bps = 1536; // kbps
	}else if(enc_size <= (960 * 576)){
		max_bps = 2048; // kbps
	}else{
		//SOC_ASSERT(0, "Bitrate limited [%dx%d]", enc_width, enc_height);
		return enc_bps;
	}

	if(enc_bps <= max_bps){
		return enc_bps;
	}

	return max_bps;
}

int32_t sdk_yuv420sem_to_bitmap888(const void *yuv420sem, int32_t yuv_width, int32_t yuv_height, int32_t yuv_stride,
	void *bmp888)
{
	int i = 0, ii = 0;

	// allocate the bitmap data
	int32_t bitmap24_size = yuv_width * yuv_height * 3;
	uint8_t *bitmap_offset = bmp888;

	//LOG_DEBUG("BMP [%dx%d] stride %d", yuv_width, yuv_height, yuv_stride);

	if(yuv420sem){
		int y, u, v, yy, vr, ug, vg, ub;
		int r, g, b;
		const uint8_t *py = (uint8_t*)(yuv420sem);
		const uint8_t *puv = (uint8_t*)(py + yuv_width * yuv_height);

		// yuv420 to rgb888
		for(i = 0; i < yuv_height; ++i){
			for(ii = 0; ii < yuv_width; ++ii){
				y = py[0];
				yy = y * 256;

				u = puv[1] - 128;
				ug = 88 * u;
				ub = 454 * u;

				v = puv[0] - 128;
				vg = 183 * v;
				vr = 359 * v;

				///////////////////////////////////
				// convert
				r = (yy + vr) >> 8;
				g = (yy - ug - vg) >> 8;
				b = (yy + ub) >> 8;

				if(r < 0){
					r = 0;
				}
				if(r > 255){
					r = 255;
				}
				if(g < 0){
					g = 0;
				}
				if(g > 255){
					g = 255;
				}
				if(b < 0){
					b = 0;
				}
				if(b > 255){
					b = 255;
				}

				*bitmap_offset++ = (uint8_t)b;
				*bitmap_offset++ = (uint8_t)g;
				*bitmap_offset++ = (uint8_t)r;

				//SOC_DEBUG("RGB[%3d,%3d,%3d] @ (%3d,%3d)", r, g, b, ii, i);

				///////////////////////////////////
				++py;
				if(0 != (ii % 2)){
					// even
					puv += 2;
				}
			}

			if(0 != (i % 2)){
				// try twice
				puv -= yuv_width;
			}
		}
		return bitmap24_size;
	}
	return -1;
}
int copy_file(char *src_name, char *des_name)
{
	int fd_old, fd_new, ret;
	char bufer[1024];
	fd_old = open(src_name, O_RDONLY);
	if(fd_old < 0)
	{
		LOG_ERR("open %s fail", src_name);
		return -1;
	}
	fd_new = open(des_name, O_WRONLY|O_TRUNC|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
	if(fd_new < 0)
	{
		LOG_ERR("open %s fail", des_name);
		close(fd_old);
		return -1;
	}
	while((ret = read(fd_old, bufer, sizeof(bufer))) > 0)
	{
		if(write(fd_new, bufer, ret) <= 0)
		{
			close(fd_old);
			close(fd_new);
			remove(des_name);
			return -1;
		}
	}
	close(fd_old);
	close(fd_new);
	return 0;
}
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>


static int is_dir(char * filename)
{
    struct stat buf;
    int ret = stat(filename,&buf);
    if(0 == ret)
    {
        if(buf.st_mode & S_IFDIR)
        {
            //printf("%s is folder\n",filename);
            return 0;
        }
        else
        {
            //printf("%s is file\n",filename);
            return 1;
        }
    }
    LOG_ERR("stat [%s] error[%s]\n", filename, strerror(errno));
    return -1;
}

int delete_path(char * path)
{
    char chBuf[128];
    DIR * dir = NULL;
    struct dirent *ptr;
    int ret = 0;

    ret = is_dir(path);
    if(1 == ret)
    {//file
        if(remove(path))
            LOG_ERR("remove %s error:%s\n", path,strerror(errno));
        return 0;
    }
    dir = opendir(path);
    if(NULL == dir)
    {
        LOG_ERR("opendir:%s error:%s\n", path,strerror(errno));
        return -1;
    }
    while((ptr = readdir(dir)) != NULL)
    {
        ret = strcmp(ptr->d_name, ".");
        if(0 == ret)
        {
            continue;
        }
        ret = strcmp(ptr->d_name, "..");
        if(0 == ret)
        {
            continue;
        }
        memset(chBuf, 0, sizeof(chBuf));
        snprintf(chBuf, sizeof(chBuf) - 1, "%s/%s", path, ptr->d_name);
        ret = is_dir(chBuf);
        if(0 == ret)
        {
            //printf("%s is dir\n", chBuf);
            ret = delete_path(chBuf);
            if(0 != ret)
            {
                LOG_ERR("delete path:%s error:%s\n", chBuf,strerror(errno));
                goto error;
            }
        }
        else if(1 == ret)
        {
            //printf("%s is file\n", chBuf);
            ret = remove(chBuf);
            if(0 != ret)
            {
                LOG_ERR("remove %s error:%s\n", path,strerror(errno));
                goto error;
            }
        }
    }
    closedir(dir);
    ret = remove(path);
    if(0 != ret)
    {
        LOG_ERR("remove %s error:%s\n", path,strerror(errno));
        char cmd[128];
        sprintf(cmd, "rm -rf %s", path);
        if(new_system_call(cmd) != 0){
            LOG_ERR("%s error:%s\n", cmd,strerror(errno));
            return -1;
        }
    }
    return 0;

error:
    closedir(dir);
    char cmd[128];
    sprintf(cmd, "rm -rf %s", path);
    if(new_system_call(cmd) != 0){
        LOG_ERR("%s error:%s\n", cmd,strerror(errno));
        return -1;
    }

    return 0;
}

int is_cjson_file(char *str)
{
    int n;
    n = strlen(str);

    if (n < 7)
        return 0;

    //判断是否是.cjson后缀
    if( str[n - 6] != '.' || str[n - 5] != 'c' || str[n - 4] != 'j' \
        || str[n - 3] != 's'  ||  str[n - 2] != 'o' || str[n - 1] != 'n')
        return 0;

    return 1;
}

int delete_cjson_file_frome_dir(char * path)
{
    //如果不是目录，则退出
    int ret = 0;
    ret = is_dir(path);
    if(1 == ret) //file
    {
        LOG_ERR("path %s is not dir\n", path);
        return 0;
    }

    //删除目录里的cjson文件
    char chBuf[128];
    DIR * dir = NULL;
    struct dirent *ptr;

    dir = opendir(path);
    if(NULL == dir)
    {
        LOG_ERR("opendir:%s error:%s\n", path,strerror(errno));
        return -1;
    }
    while((ptr = readdir(dir)) != NULL)
    {
        ret = strcmp(ptr->d_name, ".");
        if(0 == ret)
        {
            continue;
        }
        ret = strcmp(ptr->d_name, "..");
        if(0 == ret)
        {
            continue;
        }

        ret = is_cjson_file(ptr->d_name);
        if (ret == 1)
        {
            memset(chBuf, 0, sizeof(chBuf));
            snprintf(chBuf, sizeof(chBuf) - 1, "%s/%s", path, ptr->d_name);
            ret = remove(chBuf);
            if(0 != ret)
            {
                LOG_ERR("remove %s error:%s\n", path,strerror(errno));
                goto error;
            }

        }

    }
    closedir(dir);

    return 0;

error:
    closedir(dir);
    char cmd[128];
    sprintf(cmd, "rm -rf %s", path);
    if(new_system_call(cmd) != 0){
        LOG_ERR("%s error:%s\n", cmd,strerror(errno));
        return -1;
    }

    return 0;
}

