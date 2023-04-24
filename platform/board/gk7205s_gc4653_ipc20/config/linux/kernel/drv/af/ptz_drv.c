#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/slab.h>
#include<asm/uaccess.h>
#include<asm/io.h>


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/device.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <mach/hardware.h>
#include <mach/gpio.h>
#include <mach/io.h>
#include <plat/gk_gpio.h>
#include <linux/moduleparam.h>

#define	PRINTF_DEBUG	printk
#define	PRINTF_ERROR	printk

typedef enum {
	PTZ_CMD_xxx=4,
	PTZ_CMD_LEFT,
	PTZ_CMD_RIGHT,
	PTZ_CMD_UP,
	PTZ_CMD_DOWN,
	PTZ_CMD_SET_SPEED,
	PTZ_CMD_GET_SPEED,
	PTZ_ZOOM_CHECK,
	PTZ_FOCUS_CHECK,
	PTZ_CMD_STANDBY,
	PTZ_CMD_DELAY,
	PTZ_CMD_PTZ_LEFT,
	PTZ_CMD_PTZ_RIGHT,
	PTZ_CMD_PTZ_UP,
	PTZ_CMD_PTZ_DOWN,
} PTZ_CMD;

static int ptz_major = 0;

struct ptz_device {
	struct cdev *ptz_cdev;
	struct class *ptz_class;
	struct device *ptz_device;
};

typedef enum {
	MOVE_CLOCKWISE = 0,
	MOVE_ANTICLOCKWISE,
}MOVE_DIRECTION;


typedef struct  moto_dev {
    int    	aio;
    int    	bio;
    int   	cio;
    int    	dio;

	int 			run;		// 1 :run
	MOVE_DIRECTION 	direct;		// 0: clockwise/left, 1: anticlockwise
	int 			pos;		//0~7
	int 			step;
	int             step_now;

	int    is_zoom_focus;     //0: zoom 1: focus
} MOTO_DEV;

#if 0
//gos
//左右
#define GK7101_MOTO_1_A             18
#define GK7101_MOTO_1_B             19
#define GK7101_MOTO_1_C             20
#define GK7101_MOTO_1_D             21

//上下
#define GK7101_MOTO_2_A             14
#define GK7101_MOTO_2_B             15
#define GK7101_MOTO_2_C             16
#define GK7101_MOTO_2_D             17

//左右
//HZD
#define GK7101_MOTO_1_A             30
#define GK7101_MOTO_1_B             31
#define GK7101_MOTO_1_C             32
#define GK7101_MOTO_1_D             33

//上下
#define GK7101_MOTO_2_A             34
#define GK7101_MOTO_2_B             35
#define GK7101_MOTO_2_C             36
#define GK7101_MOTO_2_D             37
#endif

//focus
static int GK7101_MOTO_1_A = 24;
static int GK7101_MOTO_1_B = 27;
static int GK7101_MOTO_1_C = 28;
static int GK7101_MOTO_1_D = 36;

//zoom
static int GK7101_MOTO_2_A = 37;
static int GK7101_MOTO_2_B = 38;
static int GK7101_MOTO_2_C = 39;
static int GK7101_MOTO_2_D = 40;

//left & right
static int GK7101_MOTO_PTZ_1_A = 0;
static int GK7101_MOTO_PTZ_1_B = 0;
static int GK7101_MOTO_PTZ_1_C = 0;
static int GK7101_MOTO_PTZ_1_D = 0;

//up & down
static int GK7101_MOTO_PTZ_2_A = 0;
static int GK7101_MOTO_PTZ_2_B = 0;
static int GK7101_MOTO_PTZ_2_C = 0;
static int GK7101_MOTO_PTZ_2_D = 0;


//focus A-:2 focus A+:3 focus B-:11 focus B+:12 zoom A-:4 zoom A+:5 zoom B-:10 zoom B+:9
static int gpio_par[16];
static int int_num;
module_param_array(gpio_par,int,&int_num,S_IRUSR);

/********************************************************************/
typedef void (* HIGH_RES_TIMER_CALLBACK) (void);

extern void set_ptz_timer_call_func(HIGH_RES_TIMER_CALLBACK func);
extern void set_timer_delay_func(HIGH_RES_TIMER_CALLBACK func);
extern void set_timer_ms_delay_func(unsigned int  time);
extern void get_timer_ms_delay_func(u32 *time);

//extern int gk_set_gpio_output(struct gk_gpio_io_info *pinfo, u32 on);
extern void gk_gpio_set_out(u32 pin, u32 value);
extern void gk_gpio_config(u32 pin, u32 func /*GPIO_TYPE_OUTPUT_0*/);

/************************************************************************************************/
static struct ptz_device *fs_ptz_dev;


static MOTO_DEV 	g_moto_lr;				//left, right
static MOTO_DEV 	g_moto_upDown ;
static MOTO_DEV 	g_moto_ptz_lr;				//left, right
static MOTO_DEV 	g_moto_ptz_upDown ;

static int zoom_pos = 0;
static int focus_pos = 0;
static int zoom_direct = 0;       //0:default 1:+ 2:-
static int focus_direct = 0;	  //0:default 1+ 2:-

static spinlock_t	spinlock_ptz;
/************************************************************************************************/
static int gadi_gpio_set(int id)
{
	struct gk_gpio_io_info  gpio_info;

	gpio_info.gpio_id = id;
    gpio_info.active_level = 1;	//
    gpio_info.active_delay = 0;

//	printk("set1. gpio id=%d\n", id);
//	gk_set_gpio_output(&gpio_info, 0);
	gk_gpio_set_out(id, 1);
//	printk("set2. gpio id=%d\n", id);
	return 0;
}

static int gadi_gpio_clear(int id)
{
	struct gk_gpio_io_info  gpio_info;

	gpio_info.gpio_id = id;
    gpio_info.active_level = 0;	//
    gpio_info.active_delay = 0;

//	printk("clear1. gpio id=%d\n", id);
//	gk_set_gpio_output(&gpio_info, 0);
	gk_gpio_set_out(id, 0);
//	printk("clear2. gpio id=%d\n", id);
	return 0;
}

//
#if 0
static  int   moto_dev_run_cycle(MOTO_DEV *_dev)
{
	switch (_dev->pos) {
	case 0:
    // 1
    gadi_gpio_set(_dev->aio);
    gadi_gpio_clear(_dev->bio);
    gadi_gpio_clear(_dev->cio);
    gadi_gpio_clear(_dev->dio);
	break;

    case 1:
    // 2
    gadi_gpio_set(_dev->aio);
    gadi_gpio_set(_dev->bio);
    gadi_gpio_clear(_dev->cio);
    gadi_gpio_clear(_dev->dio);
	break;

   case 2:
    // 3
    gadi_gpio_clear(_dev->aio);
    gadi_gpio_set(_dev->bio);
    gadi_gpio_clear(_dev->cio);
    gadi_gpio_clear(_dev->dio);
	break;

    case 3:
    // 4
    gadi_gpio_clear(_dev->aio);
    gadi_gpio_set(_dev->bio);
    gadi_gpio_set(_dev->cio);
    gadi_gpio_clear(_dev->dio);
	break;

	case 4:
    //5
    gadi_gpio_clear(_dev->aio);
    gadi_gpio_clear(_dev->bio);
    gadi_gpio_set(_dev->cio);
    gadi_gpio_clear(_dev->dio);
	break;

	case 5:
    //6
    gadi_gpio_clear(_dev->aio);
    gadi_gpio_clear(_dev->bio);
    gadi_gpio_set(_dev->cio);
    gadi_gpio_set(_dev->dio);
	break;

    case 6:
    //7
    gadi_gpio_clear(_dev->aio);
    gadi_gpio_clear(_dev->bio);
    gadi_gpio_clear(_dev->cio);
    gadi_gpio_set(_dev->dio);
	break;

	case 7:
    //8
    gadi_gpio_set(_dev->aio);
    gadi_gpio_clear(_dev->bio);
    gadi_gpio_clear(_dev->cio);
    gadi_gpio_set(_dev->dio);
	break;

	default:
		PRINTF_DEBUG("%s() switch error\n", __func__);
		break;

	}

	//
	if (_dev->direct == MOVE_ANTICLOCKWISE) {
		_dev->pos++;
		if (_dev->pos > 7)
			_dev->pos = 0;
	} else {
		_dev->pos--;
		if (_dev->pos < 0 )
			_dev->pos = 7;
	}

    return 0;
}
#else
//4步驱动: 定时器2ms以上 ， 1ms跑不动
//1001 1010 0110 0101
static  int   moto_dev_run_cycle(MOTO_DEV *_dev)
{
//	printk("_dev->pos = %d\n",_dev->pos);
//	printk("write something to gpio\n");
	switch (_dev->pos) {
	case 0:
    // 1
    gadi_gpio_set(_dev->aio);
    gadi_gpio_clear(_dev->bio);
    gadi_gpio_clear(_dev->cio);
    gadi_gpio_set(_dev->dio);
	break;

    case 1:
    // 2
    gadi_gpio_set(_dev->aio);
    gadi_gpio_clear(_dev->bio);
    gadi_gpio_set(_dev->cio);
    gadi_gpio_clear(_dev->dio);
	break;

   case 2:
    // 3
    gadi_gpio_clear(_dev->aio);
    gadi_gpio_set(_dev->bio);
    gadi_gpio_set(_dev->cio);
    gadi_gpio_clear(_dev->dio);
	break;

    case 3:
    // 4
    gadi_gpio_clear(_dev->aio);
    gadi_gpio_set(_dev->bio);
    gadi_gpio_clear(_dev->cio);
    gadi_gpio_set(_dev->dio);
	break;


	default:
		PRINTF_DEBUG("%s() switch error\n", __func__);
		break;

	}

	if( _dev->is_zoom_focus == 0 )
	{
		zoom_pos = _dev->pos;
	}else if( _dev->is_zoom_focus == 1 )
	{
		focus_pos = _dev->pos;
	}

	if (_dev->direct == MOVE_ANTICLOCKWISE) {
		_dev->pos++;
		if (_dev->pos > 3)
			_dev->pos = 0;
	} else {
		_dev->pos--;
		if (_dev->pos < 0 )
			_dev->pos = 3;
	}

    return 0;
}

static  int   moto_dev_run_ptz_cycle(MOTO_DEV *_dev)
{
	switch (_dev->pos) {
	case 0:
    // 1
    gadi_gpio_set(_dev->aio);
    gadi_gpio_clear(_dev->bio);
    gadi_gpio_clear(_dev->cio);
    gadi_gpio_set(_dev->dio);
	break;

    case 1:
    // 2
    gadi_gpio_set(_dev->aio);
    gadi_gpio_set(_dev->bio);
    gadi_gpio_clear(_dev->cio);
    gadi_gpio_clear(_dev->dio);
	break;

   case 2:
    // 3
    gadi_gpio_clear(_dev->aio);
    gadi_gpio_set(_dev->bio);
    gadi_gpio_set(_dev->cio);
    gadi_gpio_clear(_dev->dio);
	break;

    case 3:
    // 4
    gadi_gpio_clear(_dev->aio);
    gadi_gpio_clear(_dev->bio);
    gadi_gpio_set(_dev->cio);
    gadi_gpio_set(_dev->dio);
	break;


	default:
		PRINTF_DEBUG("%s() switch error\n", __func__);
		break;

	}

	//
	if (_dev->direct == MOVE_ANTICLOCKWISE) {
		_dev->pos++;
		if (_dev->pos > 3)
			_dev->pos = 0;
	} else {
		_dev->pos--;
		if (_dev->pos < 0 )
			_dev->pos = 3;
	}

    return 0;
}

#endif


static int ptz_pause(MOTO_DEV *_dev)
{
	_dev->run = 0;
//	_dev->pos = 0;
	_dev->direct = 0;
	_dev->step =0 ;

/*
	//clear gpio
    gadi_gpio_clear(_dev->aio);
    gadi_gpio_clear(_dev->bio);
    gadi_gpio_clear(_dev->cio);
    gadi_gpio_clear(_dev->dio);
*/
	return 0;
}


static int ptz_move(MOTO_DEV *_dev)
{
#if 0
	static int flag = 0;
/*
1. 可以降低电流，但是速度会降低
2. 如果用定时器降速，电流会加大
	 不动		动      一圈时间
无: 5v 280mA	440mA   4S
3:  5v 280mA 	390mA 	6S
*/
	if (flag++ % 3 == 0) {
		gadi_gpio_clear(_dev->aio);
	    gadi_gpio_clear(_dev->bio);
	    gadi_gpio_clear(_dev->cio);
	    gadi_gpio_clear(_dev->dio);
		return 0;
	}

	if (flag == 300000)
		flag = 0;
#endif
    if ((g_moto_lr.run == 1) || (g_moto_upDown.run == 1)) {
        moto_dev_run_cycle(_dev);
    } else if ((g_moto_ptz_lr.run == 1) || (g_moto_ptz_upDown.run == 1)){
        moto_dev_run_ptz_cycle(_dev);
    }

//	printk("_dev->step = %d\n",_dev->step);

	_dev->step--;
	if (_dev->step <=0 ) {
//		PRINTF_DEBUG("stop move\n");
		ptz_pause(_dev);
	}

	return 0;
}

static unsigned int int_state = 0;
static unsigned int int_state_test = 0;
void ptz_timer_delay(void)
{
    spin_lock(&spinlock_ptz);
    if(0 == int_state)
        int_state++;
    //int_state_test = 0;

    spin_unlock(&spinlock_ptz);
}

void delay_ms(unsigned int time_ms)
{
    spin_lock(&spinlock_ptz);
    int_state = 0;
    //set_timer_ms_delay_func(time_ms);
    spin_unlock(&spinlock_ptz);

    while(int_state < 1)
    {
        ;//int_state_test++;
        //printk("still on delay_ms state.\n");
        set_timer_ms_delay_func(time_ms);
    }

    //printk("break delay.\n");
}
void ptz_timer_call(void)
{
	int static count=0;
	//printk("get one interrupt\n");
	if (count++ >= 1000) {
		count = 0;
	//	PRINTF_DEBUG("%s()\n", __func__);
	}

	spin_lock(&spinlock_ptz);

	if (g_moto_lr.run == 1) {
        ptz_move(&g_moto_lr);

	} else if (g_moto_upDown.run == 1) {
	    ptz_move(&g_moto_upDown);
	}

	if (g_moto_ptz_lr.run == 1) {
        ptz_move(&g_moto_ptz_lr);

	} else if (g_moto_ptz_upDown.run == 1) {
	    ptz_move(&g_moto_ptz_upDown);
	}

	spin_unlock(&spinlock_ptz);

}

/************************************************************************************************/
static int ptz_start(MOTO_DEV *_dev, PTZ_CMD cmd, unsigned int step)
{
	if ( (cmd == PTZ_CMD_LEFT) || (cmd == PTZ_CMD_UP) || (cmd == PTZ_CMD_PTZ_LEFT) || (cmd == PTZ_CMD_PTZ_UP) ) {
		_dev->direct = MOVE_ANTICLOCKWISE;
 	} else if ((cmd == PTZ_CMD_RIGHT) || (cmd == PTZ_CMD_DOWN) || (cmd == PTZ_CMD_PTZ_RIGHT) || (cmd == PTZ_CMD_PTZ_DOWN)) {
 		_dev->direct = MOVE_CLOCKWISE;
	}

//	_dev->pos = 0;
    if ( (cmd == PTZ_CMD_PTZ_RIGHT) || (cmd == PTZ_CMD_PTZ_DOWN) || (cmd == PTZ_CMD_PTZ_LEFT) || (cmd == PTZ_CMD_PTZ_UP) ) {
        _dev->pos = 0;
    }
	_dev->step = step;
	_dev->run = 1;

	return 0;
}




//
static int ptz_move_process(MOTO_DEV *_dev, PTZ_CMD cmd, int step)
{
//	PRINTF_DEBUG("start move\n");
	ptz_start(_dev, cmd, step);

//	ptz_wait(_dev, cmd, step);

	return 0;
}


static int moto_dev_init(int a, int b, int c, int d, MOTO_DEV *moto)
{
	memset(moto, 0, sizeof(MOTO_DEV));
	moto->aio = a;
	moto->bio = b;
	moto->cio = c;
	moto->dio = d;

	gk_gpio_set_out(moto->aio, GPIO_TYPE_OUTPUT_0);
	gk_gpio_set_out(moto->bio, GPIO_TYPE_OUTPUT_0);
	gk_gpio_set_out(moto->cio, GPIO_TYPE_OUTPUT_0);
	gk_gpio_set_out(moto->dio, GPIO_TYPE_OUTPUT_0);

	gadi_gpio_clear(moto->aio);
    gadi_gpio_clear(moto->bio);
    gadi_gpio_clear(moto->cio);
    gadi_gpio_clear(moto->dio);

	moto->run = 0;
	moto->pos = 0;
	moto->direct = 0;
	moto->step =0 ;
//	ptz_pause(moto);

	return 0;
}

static int moto_dev_destroy(MOTO_DEV *moto)
{
	memset(moto, 0, sizeof(MOTO_DEV));
	return 0;
}


static int ptz_open(struct inode *inode,struct file *file)
{
	spin_lock_init(&spinlock_ptz);

	memset(&g_moto_lr, 0, sizeof(MOTO_DEV));
	memset(&g_moto_upDown, 0, sizeof(MOTO_DEV));
	moto_dev_init(GK7101_MOTO_1_A,GK7101_MOTO_1_B,GK7101_MOTO_1_C,GK7101_MOTO_1_D, &g_moto_lr);
	moto_dev_init(GK7101_MOTO_2_A,GK7101_MOTO_2_B,GK7101_MOTO_2_C,GK7101_MOTO_2_D, &g_moto_upDown);

    memset(&g_moto_ptz_lr, 0, sizeof(MOTO_DEV));
	memset(&g_moto_ptz_upDown, 0, sizeof(MOTO_DEV));
    moto_dev_init(GK7101_MOTO_PTZ_1_A,GK7101_MOTO_PTZ_1_B,GK7101_MOTO_PTZ_1_C,GK7101_MOTO_PTZ_1_D, &g_moto_ptz_lr);
    moto_dev_init(GK7101_MOTO_PTZ_2_A,GK7101_MOTO_PTZ_2_B,GK7101_MOTO_PTZ_2_C,GK7101_MOTO_PTZ_2_D, &g_moto_ptz_upDown);

	set_ptz_timer_call_func(&ptz_timer_call);
    //set_timer_delay_func(&ptz_timer_delay);
	set_timer_ms_delay_func(1);

	return 0;
}
int zoom_standby=0;
int focus_standby=0;
static int ptz_standby()
{
	zoom_standby = 1;
	focus_standby = 1;

	gadi_gpio_clear(g_moto_lr.aio);
    gadi_gpio_clear(g_moto_lr.bio);
    gadi_gpio_clear(g_moto_lr.cio);
    gadi_gpio_clear(g_moto_lr.dio);

	gadi_gpio_clear(g_moto_upDown.aio);
    gadi_gpio_clear(g_moto_upDown.bio);
    gadi_gpio_clear(g_moto_upDown.cio);
    gadi_gpio_clear(g_moto_upDown.dio);
}
static int ptz_close(struct inode *inode,struct file *file)
{
	//if moving ?
	spin_lock(&spinlock_ptz);
	ptz_pause(&g_moto_lr);
	ptz_pause(&g_moto_upDown);
    ptz_pause(&g_moto_ptz_lr);
	ptz_pause(&g_moto_ptz_upDown);
	spin_unlock(&spinlock_ptz);

	moto_dev_destroy(&g_moto_lr);
	moto_dev_destroy(&g_moto_upDown);
    moto_dev_destroy(&g_moto_ptz_lr);
	moto_dev_destroy(&g_moto_ptz_upDown);

	set_ptz_timer_call_func(NULL);		//
	set_timer_ms_delay_func(1000);

//	spin_lock_uninit(spinlock_ptz);

	return 0;
}

char zoom_or_focus[1];

static void ptz_write(struct file *filp,const char __user *status, size_t count, loff_t *offset)
{
	copy_from_user(zoom_or_focus,status,count);
}

static void ptz_read(struct file *filp,const char __user *status, size_t count, loff_t *offset)
{
//	printk("g_moto_upDown.run = %d\n",g_moto_upDown.run);
	char is_runing[1]={'1'};
	char not_runing[1]={'0'};
	if(zoom_or_focus[0] == '0')//zoom
	{
		if(g_moto_lr.run == 1)
		{
			copy_to_user(status,is_runing,count);
		}else
		{
			copy_to_user(status,not_runing,count);
		}
	}

	if(zoom_or_focus[0] == '1')//focus
	{
		if(g_moto_upDown.run == 1)
		{
			copy_to_user(status,is_runing,count);
		}else
		{
			copy_to_user(status,not_runing,count);
		}
	}
}

static long ptz_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
//	PRINTF_DEBUG("%s\n", __func__);
	unsigned int step = 0;

//	step = (*(unsigned int*)arg ) << 3;
	step = (*(unsigned int*)arg );

#if 0
	if ( (g_moto_lr.run !=0) || (g_moto_upDown.run != 0) )  {
		PRINTF_DEBUG("ptz is running, g_moto_lr.run=%d, g_moto_upDown.run=%d\n", g_moto_lr.run, g_moto_upDown.run);
		return -1;
	}

#endif

	if((cmd == PTZ_CMD_LEFT)&&(cmd == PTZ_CMD_RIGHT)&&(g_moto_lr.run !=0))
	{
		printk("zoom is running, g_moto_lr.run=%d", g_moto_lr.run);
	}
	if((cmd == PTZ_CMD_UP)&&(cmd == PTZ_CMD_DOWN)&&(g_moto_lr.run !=0))
	{
		printk("focus is running, g_moto_upDown.run=%d", g_moto_upDown.run);
	}

//	if( (g_moto_lr.run !=0) && )

//	spin_lock(&spinlock_ptz);

	switch(cmd)
	{
		case PTZ_CMD_SET_SPEED: {
//			printk("PTZ_CMD_SET_SPEED CMD\n");
			unsigned int time_ms = *(unsigned int*)arg;
			set_timer_ms_delay_func(time_ms);
			break;
		}
		case PTZ_CMD_GET_SPEED: {
//			printk("PTZ_CMD_GET_SPEED CMD\n");
			unsigned int time_ms = 0;
			get_timer_ms_delay_func(&time_ms);
			*(unsigned int*)arg = time_ms;
			break;
		}

		case PTZ_CMD_LEFT: {
//			printk("PTZ_CMD_LEFT focus n CMD\n");
			g_moto_lr.is_zoom_focus = 1;

			if(focus_standby == 1)
			{
				focus_standby = 0;
				focus_pos = (focus_pos + 4 - 1)%4;
				step = step + 1;
			}

			if( focus_direct == 0)
			{
				g_moto_lr.pos = 0;
			}else if( focus_direct == 1)
			{
				g_moto_lr.pos = (focus_pos + 1)%4;
			}else if( focus_direct == 2 )
			{
				g_moto_lr.pos = (focus_pos + 1)%4;
			}

			focus_direct = 1;
//			printk("focus_pos = %d\n",focus_pos);
			ptz_move_process(&g_moto_lr, PTZ_CMD_LEFT, step);
			break;
		}
		case PTZ_CMD_RIGHT: {
//			printk("PTZ_CMD_RIGHT focus n CMD\n");
			g_moto_lr.is_zoom_focus = 1;

			if(focus_standby == 1)
			{
				focus_standby = 0;
				focus_pos = (focus_pos + 1)%4;
				step = step + 1;
			}

			if( focus_direct == 0)
			{
				g_moto_lr.pos = 0;
			}else if( focus_direct == 1)
			{
				g_moto_lr.pos = (focus_pos + 4 - 1)%4;
			}else if( focus_direct == 2 )
			{
				g_moto_lr.pos = (focus_pos + 4 - 1)%4;
			}

			focus_direct = 2;

//			printk("focus_pos = %d\n",focus_pos);
			ptz_move_process(&g_moto_lr, PTZ_CMD_RIGHT, step);
			break;
		}
		case PTZ_CMD_UP: {
//			printk("PTZ_CMD_UP zoom out CMD\n");
			g_moto_upDown.is_zoom_focus = 0;


			if(zoom_standby == 1)
			{
				zoom_standby = 0;
				zoom_pos = (zoom_pos + 4 - 1)%4;
				step = step + 1;
			}

			if( zoom_direct == 0)
			{
				g_moto_upDown.pos = 0;
			}else if( zoom_direct == 1)
			{
				g_moto_upDown.pos = (zoom_pos + 1)%4;
			}else if( zoom_direct == 2 )
			{
				g_moto_upDown.pos = (zoom_pos + 1)%4;
			}

			zoom_direct = 1;

//			printk("zoom_pos = %d\n",zoom_pos);
			ptz_move_process(&g_moto_upDown, PTZ_CMD_UP, step);
			break;
		}
		case PTZ_CMD_DOWN: {
//			printk("PTZ_CMD_DOWN zoom in CMD\n");
			g_moto_upDown.is_zoom_focus = 0;

			if(zoom_standby == 1)
			{
				zoom_standby = 0;
				zoom_pos = (zoom_pos + 1)%4;
				step = step + 1;
			}

			if( zoom_direct == 0)
			{
				g_moto_upDown.pos = 0;
			}else if( zoom_direct == 1)
			{
				g_moto_upDown.pos = (zoom_pos + 4 - 1)%4;
			}else if( zoom_direct == 2 )
			{
				g_moto_upDown.pos = (zoom_pos + 4 - 1)%4;
			}

			zoom_direct = 2;
//			printk("zoom_pos = %d\n",zoom_pos);
			ptz_move_process(&g_moto_upDown, PTZ_CMD_DOWN, step);
			break;
		}
		case PTZ_CMD_STANDBY:
//			printk("PTZ_CMD_STANDBY CMD\n");
			ptz_standby();
//			printk("standby success.\n");
			break;

        case PTZ_CMD_DELAY:{
            unsigned int time_ms = *(unsigned int*)arg;
            delay_ms(time_ms);
            break;
        }
		case PTZ_CMD_PTZ_LEFT: {
			//PRINTF_DEBUG("PTZ_CMD_PTZ_LEFT\n");
			ptz_move_process(&g_moto_ptz_lr, PTZ_CMD_PTZ_LEFT, step);
			break;
		}
		case PTZ_CMD_PTZ_RIGHT: {
			//PRINTF_DEBUG("PTZ_CMD_PTZ_RIGHT\n");
			ptz_move_process(&g_moto_ptz_lr, PTZ_CMD_PTZ_RIGHT, step);
			break;
		}
		case PTZ_CMD_PTZ_UP: {
			//PRINTF_DEBUG("PTZ_CMD_PTZ_UP\n");
			ptz_move_process(&g_moto_ptz_upDown, PTZ_CMD_PTZ_UP, step);
			break;
		}
		case PTZ_CMD_PTZ_DOWN: {
			//PRINTF_DEBUG("PTZ_CMD_PTZ_DOWN\n");
			ptz_move_process(&g_moto_ptz_upDown, PTZ_CMD_PTZ_DOWN, step);
			break;
		}
		default:{
			PRINTF_ERROR("ptz cmd error:%d\n", cmd);
			break;
		}
	}

//	spin_unlock(&spinlock_ptz);

	return 0;
}

struct file_operations ptz_fops={
	.owner		=THIS_MODULE,
	.open		=ptz_open,
	.release	=ptz_close,
	.unlocked_ioctl		=ptz_ioctl,
	.read       = ptz_read,
	.write = ptz_write,
};

static void ptz_set_cdev(void)
{
	fs_ptz_dev->ptz_cdev=cdev_alloc();
	cdev_init(fs_ptz_dev->ptz_cdev, &ptz_fops);
	fs_ptz_dev->ptz_cdev->owner =THIS_MODULE;
	cdev_add(fs_ptz_dev->ptz_cdev, MKDEV(ptz_major, 0), 1);
}


static int __init ptz_init(void)
{
	int ret;
	dev_t devno;

	if(ptz_major) {
		devno =MKDEV(ptz_major,0);
		ret =register_chrdev_region(devno, 1, "ptzs");
	} else {
		ret =alloc_chrdev_region(&devno, 0, 1, "ptzs");
		ptz_major =MAJOR(devno);
	}

	if(ret<0) {
		PRINTF_ERROR(KERN_ERR "register device major faiptz\n");
		return ret;
	}


	fs_ptz_dev =kmalloc(sizeof(struct ptz_device),GFP_KERNEL);
	if(fs_ptz_dev==NULL) {
		PRINTF_ERROR(KERN_ERR "kmalloc memory faiptz\n");
		ret =-ENOMEM;
		goto kmalloc_error;
	}


	ptz_set_cdev();


	fs_ptz_dev->ptz_class=class_create(THIS_MODULE, "ptz_class");
	if(IS_ERR(fs_ptz_dev->ptz_class)){
		PRINTF_ERROR(KERN_ERR "create class faiptz\n");
		ret =-EIO;
		goto class_create_error;
	}
	fs_ptz_dev->ptz_device=device_create(fs_ptz_dev->ptz_class, NULL,devno,NULL,"ptz");
	if(IS_ERR(fs_ptz_dev->ptz_device)){
		PRINTF_ERROR(KERN_ERR "create device faiptz\n");
		ret =-EIO;
		goto device_create_error;
	}

	//focus
	GK7101_MOTO_1_A = gpio_par[0];
	GK7101_MOTO_1_B = gpio_par[1];
	GK7101_MOTO_1_C = gpio_par[2];
	GK7101_MOTO_1_D = gpio_par[3];

	//zoom
	GK7101_MOTO_2_A = gpio_par[4];
	GK7101_MOTO_2_B = gpio_par[5];
	GK7101_MOTO_2_C = gpio_par[6];
	GK7101_MOTO_2_D = gpio_par[7];

    //left & right
	GK7101_MOTO_PTZ_1_A = gpio_par[8];
	GK7101_MOTO_PTZ_1_B = gpio_par[9];
	GK7101_MOTO_PTZ_1_C = gpio_par[10];
	GK7101_MOTO_PTZ_1_D = gpio_par[11];

	//up & down
	GK7101_MOTO_PTZ_2_A = gpio_par[12];
	GK7101_MOTO_PTZ_2_B = gpio_par[13];
	GK7101_MOTO_PTZ_2_C = gpio_par[14];
	GK7101_MOTO_PTZ_2_D = gpio_par[15];

	printk("focus A-:%d\n focus A+:%d\n focus B-:%d\n focus B+:%d\n",GK7101_MOTO_1_A, GK7101_MOTO_1_B, GK7101_MOTO_1_C, GK7101_MOTO_1_D);
	printk("zoom A-:%d\n zoom A+:%d\n zoom B-:%d\n zoom B+:%d\n",GK7101_MOTO_2_A, GK7101_MOTO_2_B, GK7101_MOTO_2_C, GK7101_MOTO_2_D);
    printk("ptz A-:%d\n ptz A+:%d\n ptz B-:%d\n ptz B+:%d\n",GK7101_MOTO_PTZ_1_A, GK7101_MOTO_PTZ_1_B, GK7101_MOTO_PTZ_1_C, GK7101_MOTO_PTZ_1_D);
    printk("ptz A-:%d\n ptz A+:%d\n ptz B-:%d\n ptz B+:%d\n",GK7101_MOTO_PTZ_2_A, GK7101_MOTO_PTZ_2_B, GK7101_MOTO_PTZ_2_C, GK7101_MOTO_PTZ_2_D);

	return 0;

device_create_error:
	class_destroy(fs_ptz_dev->ptz_class);
class_create_error:
	kfree(fs_ptz_dev);
	cdev_del(fs_ptz_dev->ptz_cdev);
kmalloc_error:
	unregister_chrdev_region(MKDEV(ptz_major, 0),1);
	return ret;
}


static void __exit ptz_exit(void)
{
	unregister_chrdev_region(MKDEV(ptz_major, 0),1);
	device_destroy(fs_ptz_dev->ptz_class, MKDEV(ptz_major, 0));
	class_destroy(fs_ptz_dev->ptz_class);
	cdev_del(fs_ptz_dev->ptz_cdev);
	kfree(fs_ptz_dev);
}


module_init(ptz_init);
module_exit(ptz_exit);

MODULE_LICENSE("GPL");
