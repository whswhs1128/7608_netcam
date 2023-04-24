/**
 * dwc3-goke.c Support for dwc3 platform devices on Goke Microelectronics platforms
 *
 * This is a small driver for the dwc3 to provide the glue logic
 * to configure the controller. Tested on GKIPC platforms.
 *
 * Copyright (C) 2017, goke, Inc.
 *
 * Author: Author: Steven Yu, <yulindeng@goke.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Inspired by dwc3-omap.c and dwc3-exynos.c.
 */

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
//#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/regmap.h>
//#include <linux/reset.h>
#include <linux/pinctrl/consumer.h>
//#include <linux/usb/of.h>

#include "core.h"
#include "io.h"
#include "gk_usb_phy.h"
#include <plat/gk_gpio.h>
#include CONFIG_GK_CHIP_INCLUDE_FILE

/* glue registers */
#define CLKRGK_CTRL             0x00
#define AUX_CLK_EN              BIT(0)
#define SW_PIPEW_RESET_N        BIT(4)
#define EXT_CFG_RESET_N         BIT(8)
/*
 * 1'b0 : The host controller complies with the xHCI revision 0.96
 * 1'b1 : The host controller complies with the xHCI revision 1.0
 */
#define XHCI_REVISION           BIT(12)

#define USB2_VBUS_MNGMNT_SEL1   0x2C
/*
 * For all fields in USB2_VBUS_MNGMNT_SEL1
 * 2’b00 : Override value from Reg 0x30 is selected
 * 2’b01 : utmiotg_<signal_name> from usb3_top is selected
 * 2’b10 : pipew_<signal_name> from PIPEW instance is selected
 * 2’b11 : value is 1'b0
 */
#define USB2_VBUS_REG30         0x0
#define USB2_VBUS_UTMIOTG       0x1
#define USB2_VBUS_PIPEW         0x2
#define USB2_VBUS_ZERO          0x3

#define SEL_OVERRIDE_VBUSVALID(n)       (n << 0)
#define SEL_OVERRIDE_POWERPRESENT(n)    (n << 4)
#define SEL_OVERRIDE_BVALID(n)          (n << 8)

/* Static DRD configuration */
#define USB3_CONTROL_MASK       0xf77

#define USB3_DEVICE_NOT_HOST        BIT(0)
#define USB3_FORCE_VBUSVALID        BIT(1)
#define USB3_DELAY_VBUSVALID        BIT(2)
#define USB3_SEL_FORCE_OPMODE       BIT(4)
#define USB3_FORCE_OPMODE(n)        (n << 5)
#define USB3_SEL_FORCE_DPPULLDOWN2  BIT(8)
#define USB3_FORCE_DPPULLDOWN2      BIT(9)
#define USB3_SEL_FORCE_DMPULLDOWN2  BIT(10)
#define USB3_FORCE_DMPULLDOWN2      BIT(11)

/**
 * struct gk_dwc3 - dwc3-gk driver private structure
 * @dev:            device pointer
 * @base:           ioaddr for the glue registers
 * @regmap:         regmap pointer for getting syscfg
 * @syscfg_reg_off: usb syscfg control offset
 * @dr_mode:        drd static host/device config
 * @rstc_pwrdn:     rest controller for powerdown signal
 * @rstc_rst:       reset controller for softreset signal
 */

struct gk_dwc3_glue {
    /* device lock */
    spinlock_t              lock;
    struct device*          dev;
    struct platform_device* dwc3;
    void __iomem*           base;
    struct regmap*          regmap;
    int syscfg_reg_off;
    //enum usb_dr_mode dr_mode;
    //struct reset_control *rstc_pwrdn;
    //struct reset_control *rstc_rst;
};


static inline u32 gk_dwc3_readl(void __iomem *base, u32 offset)
{
   //return readl_relaxed(base + offset);
     return gk_usb_readl((u32)base + offset);
}

static inline void gk_dwc3_writel(void __iomem *base, u32 offset, u32 value)
{
   gk_usb_writel((u32)base + offset, value);
     //writel_relaxed(value, base + offset);
}

#if 0
/**
 * gk_dwc3_drd_init: program the port
 * @dwc3_data: driver private structure
 * Description: this function is to program the port as either host or device
 * according to the static configuration passed from devicetree.
 * OTG and dual role are not yet supported!
 */
static int gk_dwc3_drd_init(struct gk_dwc3_glue *dwc3_data)
{
    u32 val;
    int err;

    err = regmap_read(dwc3_data->regmap, dwc3_data->syscfg_reg_off, &val);
    if (err)
        return err;

    val &= USB3_CONTROL_MASK;
    return regmap_write(dwc3_data->regmap, dwc3_data->syscfg_reg_off, val);
}

/**
 * gk_dwc3_init: init the controller via glue logic
 * @dwc3_data: driver private structure
 */
static void gk_dwc3_init(struct gk_dwc3_glue *dwc3_data)
{
    u32 reg = gk_dwc3_readl(dwc3_data->base, CLKRGK_CTRL);
}
#endif

static void GH_USB_PHY_init(void)
{
	int write_flag=0;
	
    printk("[GKN]FILE:%s, LINE:%d, %s()\n", __FILE__, __LINE__, __FUNCTION__);

	for(write_flag=0;write_flag<2;write_flag++)
	{
		gk_usb_writew(REG_USB_PHY_UTMI_REG_1D, ((u16)0x00006000));
		gk_usb_writew(REG_USB_PHY_UTMI_REG_12, ((u16)0x00000060));
		msleep(1);
	    gk_usb_writew(REG_USB_PHY_UTMI0, ((u16)0x0000ff05));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_01, ((u16)0x00009080));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_02, ((u16)0x00003000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_03, ((u16)0x00003020));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_04, ((u16)0x00000483));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_05, ((u16)0x00008044));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_06, ((u16)0x000000c0));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_08, ((u16)0x00000000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_09, ((u16)0x000000ff));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_0A, ((u16)0x0000000b));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_0B, ((u16)0x00000000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_0C, ((u16)0x000023cc));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_10, ((u16)0x00000000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_11, ((u16)0x00000000));
	    
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_13, ((u16)0x00000000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_14, ((u16)0x00000000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_15, ((u16)0x00000000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_16, ((u16)0x00000000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_17, ((u16)0x00000000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_18, ((u16)0x00000000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_19, ((u16)0x00000000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_1A, ((u16)0x00000000));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_1B, ((u16)0x00000001));
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_1C, ((u16)0x00002801));
	    
	    gk_usb_writew(REG_USB_PHY_UTMI_REG_1E, ((u16)0x00000000));
	}

}

static int gk_usb_init(void)
{
    struct gk_gpio_io_info  usb_gpio_conf;
    usb_gpio_conf.gpio_id = gk_all_gpio_cfg.usb_host;
    usb_gpio_conf.active_level = GPIO_HIGH;
    usb_gpio_conf.active_delay = 1;

    printk("USB phy Begin initial sequence ...\n");

#ifdef CONFIG_ARCH_GK720X
    GH_USB_PHY_init();
#endif

    gk_rct_setbitsl(GK_VA_RCT + 0x50, 0x2);         //0xA0170050
    msleep(2);
    gk_rct_clrbitsl(GK_VA_RCT + 0x50, 0x2);
    msleep(2);
    gk_rct_setbitsl(GK_VA_RCT + 0x50, 0x2);

#ifdef CONFIG_ARCH_GK720X_FPGA
    printk("Usb External PHY Reset...  \n");
    gk_set_gpio_output(&usb_gpio_conf, 1);
    mdelay(2);
    gk_set_gpio_output(&usb_gpio_conf, 0);
    mdelay(2);
#endif

    printk("USB Controller Software Reset...  \n");
    // usb software rst
    gk_rct_setbitsl(GK_VA_RCT + 0x88, 0x20000000);   // 0xA0170088
    msleep(2);
    gk_rct_clrbitsl(GK_VA_RCT + 0x88, 0x20000000);

#ifdef CONFIG_ARCH_GK720X
    // init UTMI
    gk_usb_writel((REG_UTMI_BASE+(0x04<<2)), 0x0000040f);
    gk_usb_writel((REG_UTMI_BASE+(0x10<<2)), 0x00008051);
    gk_usb_writel((REG_UTMI_BASE+(0x11<<2)), 0x00005080);
    gk_usb_writel((REG_UTMI_BASE+(0x11<<2)), 0x00005090);
    gk_usb_writel((REG_UTMI_BASE+(0x00<<2)), 0x00006bc3);
    gk_usb_writel((REG_UTMI_BASE+(0x0A<<2)), 0x00003403);
    mdelay(20);
    gk_usb_writel((REG_UTMI_BASE+(0x00<<2)), 0x000069c3);
    mdelay(20);
    gk_usb_writel((REG_UTMI_BASE+(0x11<<2)), 0x00005080);
    mdelay(20);
    gk_usb_writel((REG_UTMI_BASE+(0x00<<2)), 0x00000001);
    mdelay(20);
    gk_usb_writel((REG_UTMI_BASE+(0x1e<<2)), 0x00000001);
    mdelay(20);
    gk_usb_writel((REG_UTMI_BASE+(0x1e<<2)), 0x00000000);
    mdelay(20);
    gk_usb_writel((REG_UTMI_BASE+(0x03<<2)), 0x00000023);
    mdelay(20);
    gk_usb_writel((REG_UTMI_BASE+(0x03<<2)), 0x00000020);
    gk_usb_writel((REG_UTMI_BASE+(0x0a<<2)), 0x00000003);
#if defined(CONFIG_USB_DWC3_HOST)
    gk_usb_writel((REG_UTMI_BASE+(0x0a<<2)), 0x00000103);
#elif defined(CONFIG_USB_DWC3_DUAL_ROLE)
    if (!gk_gpio_get(CONFIG_GK_USB_OTG_GPIO))
    {
        gk_usb_writel((REG_UTMI_BASE+(0x0a<<2)), 0x00000103);
    }
    else
    {
        gk_usb_writel((REG_UTMI_BASE+(0x0a<<2)), 0x0000010B);
    }
#elif defined(CONFIG_USB_DWC3_GADGET)
    gk_usb_writel((REG_UTMI_BASE+(0x0a<<2)), 0x0000010B);
#endif

    mdelay(20);
    gk_usb_writel((REG_UTMI_BASE+(0x03<<2)), 0x00000023);
    mdelay(20);
    gk_usb_writel((REG_UTMI_BASE+(0x03<<2)), 0x00000020);
    mdelay(20);
/*
  // by GK-D, 05/16/2018
    //reset USB controller
    gk_rct_writel(GK_VA_RCT + 0x0088, 0x6CC36011);
    mdelay(20);
    gk_rct_writel(GK_VA_RCT + 0x0088, 0x4CC36011);
    mdelay(20);
*/
/*
    printk("USB Controller Software Reset...  \n");
    // usb software rst
    gk_rct_setbitsl(GK_VA_RCT + 0x88, 0x20000000);   // 0xA0170088
    msleep(20);
    gk_rct_clrbitsl(GK_VA_RCT + 0x88, 0x20000000);
*/
#endif

    return 0;
}

void gk_utmi_reg_hold_phy(int set)
{
    u32  value = 0;

    value = gk_usb_readl(REG_UTMI_BASE+ 0x28);
    printk("%s set %d before regvalue %x\n",__func__,set,value);
    if(set == 1){ 
        value = value | 0x200; 
        value = value & ~(0x100); 
    }   
    else {
        value = value &(~0x200); 
        value = value | 0x100; 
    }   
    gk_usb_writel((REG_UTMI_BASE+ 0x28), value);
}

static ssize_t show_phyreset(struct device *dev, struct device_attribute *attr, char *buf)
{
    int value = 0;
    printk("do nothing\n");
    return sprintf(buf, "%d\n", value);
}

extern void dwc3_controller_remove_soft(void);
extern void dwc3_controller_probe_soft(void);
static ssize_t set_phyreset(struct device *dev, struct device_attribute *attr,
        const char *buf, size_t count)
{
    int value;

    if (sscanf(buf, "%d", &value) != 1)
        return -EINVAL;

    if (value == 3) {
        dwc3_controller_remove_soft();
        gk_usb_init();
        dwc3_controller_probe_soft();
        printk("will reinit phy & reset controller \n");
    }

    return count;
}

static DEVICE_ATTR(phyreset, S_IRUGO | S_IWUSR, show_phyreset, set_phyreset);
static int gk_dwc3_probe(struct platform_device *pdev)
{
    int ret = -ENOMEM;
    //struct musb_hdrc_platform_data  *pdata = pdev->dev.platform_data;
    struct gk_dwc3_glue *glue;
    struct resource *res;
    struct device *dev = &pdev->dev;
    struct device_node *node = dev->of_node, *child;
    struct platform_device *dwc3;
    struct regmap *regmap;

    printk("[%s %s %d]\n", __FILE__, __func__, __LINE__);
    gk_usb_init();
    glue = devm_kzalloc(dev, sizeof(*glue), GFP_KERNEL);
    if (!glue)
        return -ENOMEM;

    dwc3 = platform_device_alloc("dwc3", -1);
    if (!dwc3) {
        dev_err(&pdev->dev, "failed to allocate dwc3 device\n");
        goto err1;
    }

    dwc3->dev.parent            = &pdev->dev;
    dwc3->dev.dma_mask          = &gk_dmamask;
    dwc3->dev.coherent_dma_mask = gk_dmamask;

    glue->dev       = dev;
    glue->dwc3      = dwc3;
    glue->regmap    = regmap;

    //pdata->platform_ops         = &gk_ops;

	spin_lock_init(&glue->lock);
    platform_set_drvdata(pdev, glue);

    ret = platform_device_add_resources(dwc3, pdev->resource,
            pdev->num_resources);
    if (ret) {
        dev_err(&pdev->dev, "failed to add resources\n");
        goto err2;
    }

    //ret = platform_device_add_data(dwc3, pdata, sizeof(*pdata));
    if (ret) {
        dev_err(&pdev->dev, "failed to add platform_data\n");
        goto err2;
    }

    ret = platform_device_add(dwc3);
    if (ret) {
        dev_err(&pdev->dev, "failed to register dwc3 device\n");
        goto err1;
    }
    sysfs_add_file_to_group(&pdev->dev.kobj,
                        &dev_attr_phyreset.attr, NULL);
    printk("platform add gk dwc3...\n");
    return 0;

err2:
    platform_device_put(dwc3);
err1:
    kfree(glue);
err0:
    return ret;
}

static int gk_dwc3_remove(struct platform_device *pdev)
{
    struct gk_dwc3_glue *glue = platform_get_drvdata(pdev);
    printk("[%s %s %d]\n", __FILE__, __func__, __LINE__);

    platform_device_del(glue->dwc3);
    platform_device_put(glue->dwc3);

    pdev->resource[0].parent = NULL;
    pdev->resource[0].sibling = NULL;
    pdev->resource[0].child = NULL;
    pdev->resource[1].parent = NULL;
    pdev->resource[1].sibling = NULL;
    pdev->resource[1].child = NULL;
    kfree(glue);

    return 0;
}
   
static struct platform_driver gk_dwc3_driver =
{
    .probe      = gk_dwc3_probe,
    .remove     = gk_dwc3_remove,
    .driver     =
    {
        .name   = "gk_dwc3",
    },
};

static int __init gk_init(void)
{
    printk("[%s %s %d]\n", __FILE__, __func__, __LINE__);
    return platform_driver_register(&gk_dwc3_driver);
}
module_init(gk_init);

static void __exit gk_exit(void)
{
    printk("[%s %s %d]\n", __FILE__, __func__, __LINE__);
    platform_driver_unregister(&gk_dwc3_driver);
}
module_exit(gk_exit);

MODULE_DESCRIPTION("GOKE USB3 Glue Layer");
MODULE_AUTHOR("Goke Microelectronics Inc.");
MODULE_LICENSE("GPL v2");

