/*!
*****************************************************************************
** \file        arch/arm/include/asm/goke-common/gk_gpio_t26_r25.h
**
** \version     $Id$
**
** \brief
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2016 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _GK_GPIO_T26_R25_H_
#define _GK_GPIO_T26_R25_H_
/*!
*****************************************************************************
** CONFIG_MACH_GK_T26_R25_A used for GK7202_FPGA
** CONFIG_MACH_GK_T26_R25_B used for GK7205_EVB
*****************************************************************************
*/
#define SYSTEM_GPIO_UART_TABLE             \
    { GPIO_0 , GK_GPIO_SF_CS0           }, \
    { GPIO_26, GK_GPIO_UART0_TX         }, \
    { GPIO_25, GK_GPIO_UART0_RX         }

#if defined(CONFIG_MACH_GK_T26_R25_A)
#define SYSTEM_GPIO_EXTPHY_TABLE           \
    { GPIO_32, GK_GPIO_EPHY_LED_1       },

#define SYSTEM_GPIO_INTPHY_TABLE           \
    { GPIO_32, GK_GPIO_EPHY_LED_1       },

#define SYSTEM_GPIO_XREF_TABLE             \
    { GPIO_0 , GK_GPIO_SF_CS0           }, \
    { GPIO_1 , GK_GPIO_ENET_PHY_RXD_0   }, \
    { GPIO_2 , GK_GPIO_ENET_PHY_RXD_1   }, \
    { GPIO_3 , GK_GPIO_ENET_PHY_RXD_2   }, \
    { GPIO_4 , GK_GPIO_ENET_PHY_RXD_3   }, \
    { GPIO_5 , GK_GPIO_UNDEFINED        }, \
    { GPIO_6 , GK_GPIO_UNDEFINED        }, \
    { GPIO_7 , GK_GPIO_UNDEFINED        }, \
    { GPIO_8 , GK_GPIO_UNDEFINED        }, \
    { GPIO_9 , GK_GPIO_UNDEFINED        }, \
    { GPIO_10, GK_GPIO_UNDEFINED        }, \
    { GPIO_11, GK_GPIO_UNDEFINED        }, \
    { GPIO_12, GK_GPIO_UNDEFINED        }, \
    { GPIO_13, GK_GPIO_UNDEFINED        }, \
    { GPIO_14, GK_GPIO_UNDEFINED        }, \
    { GPIO_15, GK_GPIO_UNDEFINED        }, \
    { GPIO_16, GK_GPIO_UNDEFINED        }, \
    { GPIO_17, GK_GPIO_UNDEFINED        }, \
    { GPIO_18, GK_GPIO_UNDEFINED        }, \
    { GPIO_19, GK_GPIO_UNDEFINED        }, \
    { GPIO_20, GK_GPIO_UNDEFINED        }, \
    { GPIO_21, GK_GPIO_UNDEFINED        }, \
    { GPIO_22, GK_GPIO_UNDEFINED        }, \
    { GPIO_23, GK_GPIO_ENET_PHY_CLK_RX  }, \
    { GPIO_24, GK_GPIO_ENET_PHY_CLK_TX  }, \
    { GPIO_25, GK_GPIO_UART0_RX         }, \
    { GPIO_26, GK_GPIO_UART0_TX         }, \
    { GPIO_27, GK_GPIO_ENET_PHY_TXD_0   }, \
    { GPIO_28, GK_GPIO_ENET_PHY_TXD_1   }, \
    { GPIO_29, GK_GPIO_ENET_PHY_TXD_2   }, \
    { GPIO_30, GK_GPIO_ENET_PHY_TXD_3   }, \
    { GPIO_31, GK_GPIO_UNDEFINED        }, \
    { GPIO_32, GK_GPIO_UNDEFINED        }, \
    { GPIO_33, GK_GPIO_UNDEFINED        }, \
    { GPIO_34, GK_GPIO_UNDEFINED        }, \
    { GPIO_35, GK_GPIO_UNDEFINED        }, \
    { GPIO_36, GK_GPIO_ENET_PHY_TXEN    }, \
    { GPIO_37, GK_GPIO_PHY_RESET        }, \
    { GPIO_38, GK_GPIO_ENET_PHY_MDC     }, \
    { GPIO_39, GK_GPIO_ENET_PHY_MDIO    }, \
    { GPIO_40, GK_GPIO_ENET_PHY_CRS     }, \
    { GPIO_41, GK_GPIO_ENET_PHY_RXER    }, \
    { GPIO_42, GK_GPIO_UNDEFINED        }, \
    { GPIO_43, GK_GPIO_UNDEFINED        }, \
    { GPIO_44, GK_GPIO_UNDEFINED        }, \
    { GPIO_45, GK_GPIO_ENET_PHY_RXDV    }, \
    { GPIO_46, GK_GPIO_UNDEFINED        }, \
    { GPIO_47, GK_GPIO_UNDEFINED        }, \
    { GPIO_48, GK_GPIO_UNDEFINED        }, \
    { GPIO_49, GK_GPIO_UNDEFINED        }, \
    { GPIO_50, GK_GPIO_UNDEFINED        }, \
    { GPIO_51, GK_GPIO_UNDEFINED        }, \
    { GPIO_52, GK_GPIO_UNDEFINED        }, \
    { GPIO_53, GK_GPIO_UNDEFINED        }, \
    { GPIO_54, GK_GPIO_UNDEFINED        }, \
    { GPIO_55, GK_GPIO_UNDEFINED        }, \
    { GPIO_56, GK_GPIO_UNDEFINED        }, \
    { GPIO_57, GK_GPIO_UNDEFINED        }, \
    { GPIO_58, GK_GPIO_UNDEFINED        }, \
    { GPIO_59, GK_GPIO_UNDEFINED        }, \
    { GPIO_60, GK_GPIO_UNDEFINED        }, \
    { GPIO_61, GK_GPIO_SENSOR_RESET     }, \
    { GPIO_62, GK_GPIO_I2C0_DATA        }, \
    { GPIO_63, GK_GPIO_I2C0_CLK         }, \
    { GPIO_64, GK_GPIO_ENET_PHY_COL     }, \
    { GPIO_65, GK_GPIO_UNDEFINED        }, \
    { GPIO_66, GK_GPIO_UNDEFINED        }, \
    { GPIO_67, GK_GPIO_UNDEFINED        }, \
    { GPIO_68, GK_GPIO_UNDEFINED        }, \
    { GPIO_69, GK_GPIO_UNDEFINED        }, \
    { GPIO_70, GK_GPIO_UNDEFINED        }, \
    { GPIO_71, GK_GPIO_UNDEFINED        }, \
    { GPIO_72, GK_GPIO_UNDEFINED        }, \
    { GPIO_73, GK_GPIO_UNDEFINED        }, \
    { GPIO_74, GK_GPIO_UNDEFINED        }, \
    { GPIO_75, GK_GPIO_UNDEFINED        }, \
    { GPIO_76, GK_GPIO_UNDEFINED        }, \
    { GPIO_77, GK_GPIO_UNDEFINED        }, \
    { GPIO_78, GK_GPIO_UNDEFINED        }, \
    { GPIO_79, GK_GPIO_UNDEFINED        },
#endif /* CONFIG_MACH_GK_T26_R25_A */


#if defined(CONFIG_MACH_GK_T26_R25_B)
#if defined(CONFIG_SYSTEM_USE_EXTERN_I2S_FOR7205)
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_4 , GK_GPIO_AO0_BCLK         }, \
    { GPIO_5 , GK_GPIO_AO0_LRCLK        }, \
    { GPIO_6 , GK_GPIO_UNDEFINED        }, \
    { GPIO_7 , GK_GPIO_UNDEFINED        }, \
    { GPIO_8 , GK_GPIO_AO0_MCLK         }, \
    { GPIO_9 , GK_GPIO_AO0_DATA         }, \
    { GPIO_10, GK_GPIO_AO0_DATA1        }, \
    { GPIO_11, GK_GPIO_AI0_BCLK         }, \
    { GPIO_14, GK_GPIO_UNDEFINED        }, \
    { GPIO_15, GK_GPIO_UNDEFINED        }, \
    { GPIO_16, GK_GPIO_AI0_DATA         }, \
    { GPIO_17, GK_GPIO_IR_CUT1          }, \
    { GPIO_18, GK_GPIO_UNDEFINED        }, \
    { GPIO_19, GK_GPIO_UNDEFINED        }, \
    { GPIO_20, GK_GPIO_UNDEFINED        }, \
    { GPIO_22, GK_GPIO_AI0_DATA1        },
#elif defined(CONFIG_SYSTEM_USE_EXTERN_I2S_FOR7202)
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_4 , GK_GPIO_AO0_BCLK         }, \
    { GPIO_5 , GK_GPIO_AO0_LRCLK        }, \
    { GPIO_6 , GK_GPIO_UNDEFINED        }, \
    { GPIO_7 , GK_GPIO_UNDEFINED        }, \
    { GPIO_8 , GK_GPIO_UNDEFINED        }, \
    { GPIO_9 , GK_GPIO_UNDEFINED        }, \
    { GPIO_10, GK_GPIO_UNDEFINED        }, \
    { GPIO_11, GK_GPIO_AO0_MCLK         }, \
    { GPIO_12, GK_GPIO_AO0_DATA         }, \
    { GPIO_13, GK_GPIO_AO0_DATA1        }, \
    { GPIO_14, GK_GPIO_UNDEFINED        }, \
    { GPIO_15, GK_GPIO_UNDEFINED        }, \
    { GPIO_16, GK_GPIO_IR_CUT1          }, \
    { GPIO_17, GK_GPIO_IR_CUT1          }, \
    { GPIO_18, GK_GPIO_UNDEFINED        }, \
    { GPIO_19, GK_GPIO_UNDEFINED        }, \
    { GPIO_20, GK_GPIO_UNDEFINED        }, \
    { GPIO_22, GK_GPIO_AI0_DATA         }, \
    { GPIO_23, GK_GPIO_AI0_DATA1        },
#elif defined(CONFIG_SYSTEM_USE_SDCARD)
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_2 , GK_GPIO_SDIO1_WP_N       }, \
    { GPIO_3 , GK_GPIO_SDIO1_DATA_1     }, \
    { GPIO_4 , GK_GPIO_SDIO1_DATA_0     }, \
    { GPIO_5 , GK_GPIO_SDIO1_CLK        }, \
    { GPIO_9 , GK_GPIO_SDIO1_CMD        }, \
    { GPIO_10 , GK_GPIO_SDIO1_DATA_3     }, \
    { GPIO_11, GK_GPIO_SDIO1_DATA_2     }, \
    { GPIO_12, GK_GPIO_SDIO1_CD_N       }, \
    { GPIO_13, GK_GPIO_SDIO0_WP_N       }, \
    { GPIO_14, GK_GPIO_SDIO0_DATA_1     }, \
    { GPIO_15, GK_GPIO_SDIO0_DATA_0     }, \
    { GPIO_19, GK_GPIO_SDIO0_CLK        }, \
    { GPIO_20, GK_GPIO_SDIO0_CMD        }, \
    { GPIO_21, GK_GPIO_SDIO0_DATA_3     }, \
    { GPIO_22, GK_GPIO_SDIO0_DATA_2     }, \
    { GPIO_23, GK_GPIO_SDIO0_CD_N       },

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
    { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
    { GPIO_6 , GK_GPIO_UNDEFINED        }, \
    { GPIO_7 , GK_GPIO_UNDEFINED        }, \
    { GPIO_8 , GK_GPIO_UNDEFINED        }, \
    { GPIO_9 , GK_GPIO_UNDEFINED        }, \
    { GPIO_10, GK_GPIO_UNDEFINED        }, \
    { GPIO_11, GK_GPIO_UNDEFINED        }, \
    { GPIO_14, GK_GPIO_UNDEFINED        }, \
    { GPIO_15, GK_GPIO_UNDEFINED        }, \
    { GPIO_16, GK_GPIO_IR_CUT1          }, \
    { GPIO_17, GK_GPIO_IR_CUT1          }, \
    { GPIO_18, GK_GPIO_UNDEFINED        }, \
    { GPIO_19, GK_GPIO_UNDEFINED        }, \
    { GPIO_20, GK_GPIO_UNDEFINED        }, \
    { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
    { GPIO_2 , GK_GPIO_UNDEFINED        }, \
    { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
    { GPIO_2,  GK_GPIO_PHY_DATA_LED     }, \
    { GPIO_3,  GK_GPIO_PHY_LINK_LED     },

#define SYSTEM_GPIO_XREF_TABLE             \
    { GPIO_0 , GK_GPIO_SF_CS0           }, \
    { GPIO_1 , GK_GPIO_SF_CS1       }, \
    SYSTEM_GPIO_SDCARD_TABLE\
    { GPIO_21, GK_GPIO_UNDEFINED        }, \
    { GPIO_24, GK_GPIO_UNDEFINED        }, \
    { GPIO_25, GK_GPIO_UART0_RX         }, \
    { GPIO_26, GK_GPIO_UART0_TX         }, \
    { GPIO_27, GK_GPIO_UNDEFINED        }, \
    { GPIO_28, GK_GPIO_UNDEFINED        }, \
    { GPIO_29, GK_GPIO_SF_HOLD        }, \
    { GPIO_30, GK_GPIO_SF_WP        }, \
    { GPIO_31, GK_GPIO_UNDEFINED        }, \
    { GPIO_32, GK_GPIO_UNDEFINED        }, \
    { GPIO_33, GK_GPIO_UNDEFINED        }, \
    { GPIO_34, GK_GPIO_UNDEFINED        }, \
    { GPIO_35, GK_GPIO_UNDEFINED        }, \
    { GPIO_36, GK_GPIO_SPI0_SCLK        }, \
    { GPIO_37, GK_GPIO_SPI0_SO          }, \
    { GPIO_38, GK_GPIO_SPI0_SI          }, \
    { GPIO_39, GK_GPIO_UNDEFINED        }, \
    { GPIO_40, GK_GPIO_UNDEFINED        }, \
    { GPIO_41, GK_GPIO_UNDEFINED        }, \
    { GPIO_42, GK_GPIO_UNDEFINED        }, \
    { GPIO_43, GK_GPIO_UNDEFINED        }, \
    { GPIO_44, GK_GPIO_SENSOR1_RESET    }, \
    { GPIO_45, GK_GPIO_UNDEFINED        }, \
    { GPIO_46, GK_GPIO_UNDEFINED        }, \
    { GPIO_47, GK_GPIO_UNDEFINED        }, \
    { GPIO_48, GK_GPIO_UNDEFINED        }, \
    { GPIO_49, GK_GPIO_UNDEFINED        }, \
    { GPIO_50, GK_GPIO_UNDEFINED        }, \
    { GPIO_51, GK_GPIO_UNDEFINED        }, \
    { GPIO_52, GK_GPIO_UNDEFINED        }, \
    { GPIO_53, GK_GPIO_UNDEFINED        }, \
    { GPIO_54, GK_GPIO_UNDEFINED        }, \
    { GPIO_55, GK_GPIO_UNDEFINED        }, \
    { GPIO_56, GK_GPIO_UNDEFINED        }, \
    { GPIO_57, GK_GPIO_UNDEFINED        }, \
    { GPIO_58, GK_GPIO_UNDEFINED        }, \
    { GPIO_59, GK_GPIO_UNDEFINED        }, \
    { GPIO_60, GK_GPIO_UNDEFINED        }, \
    { GPIO_61, GK_GPIO_I2C0_DATA        }, \
    { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
    { GPIO_63, GK_GPIO_I2C0_CLK         }, \
    { GPIO_64, GK_GPIO_UNDEFINED        }, \
    { GPIO_65, GK_GPIO_UNDEFINED        }, \
    { GPIO_66, GK_GPIO_UNDEFINED        }, \
    { GPIO_67, GK_GPIO_UNDEFINED        }, \
    { GPIO_68, GK_GPIO_UNDEFINED        }, \
    { GPIO_69, GK_GPIO_UNDEFINED        }, \
    { GPIO_70, GK_GPIO_UNDEFINED        }, \
    { GPIO_71, GK_GPIO_UNDEFINED        }, \
    { GPIO_72, GK_GPIO_UNDEFINED        }, \
    { GPIO_73, GK_GPIO_UNDEFINED        }, \
    { GPIO_74, GK_GPIO_UNDEFINED        }, \
    { GPIO_75, GK_GPIO_UNDEFINED        }, \
    { GPIO_76, GK_GPIO_UNDEFINED        }, \
    { GPIO_77, GK_GPIO_UNDEFINED        }, \
    { GPIO_78, GK_GPIO_UNDEFINED        }, \
    { GPIO_79, GK_GPIO_UNDEFINED        },
#endif /* CONFIG_MACH_GK_T26_R25_B */

#if defined(CONFIG_MACH_GK_T26_R25_B2)

#if CONFIG_SYSTEM_VOUT == 1       //I80 8bit(OK)
#define SYSTEM_GPIO_SDCARD_TABLE        \
        { GPIO_14, GK_GPIO_SDIO0_WP_N       }, \
        { GPIO_15, GK_GPIO_SDIO0_DATA_1     }, \
        { GPIO_16, GK_GPIO_SDIO0_DATA_0     }, \
        { GPIO_17, GK_GPIO_SDIO0_CMD        }, \
        { GPIO_18, GK_GPIO_SDIO0_DATA_3     }, \
        { GPIO_19, GK_GPIO_SDIO0_DATA_2     }, \
        { GPIO_20, GK_GPIO_SDIO0_CD_N       }, \
        { GPIO_22, GK_GPIO_SDIO0_CLK        },
#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
        { GPIO_14, GK_GPIO_UNDEFINED        }, \
        { GPIO_15, GK_GPIO_UNDEFINED        }, \
        { GPIO_16, GK_GPIO_UNDEFINED        }, \
        { GPIO_17, GK_GPIO_UNDEFINED        }, \
        { GPIO_18, GK_GPIO_UNDEFINED        }, \
        { GPIO_19, GK_GPIO_UNDEFINED        }, \
        { GPIO_20, GK_GPIO_UNDEFINED        }, \
        { GPIO_22, GK_GPIO_UNDEFINED        },
#endif/* SYSTEM_GPIO_SDCARD_TABLE */

#define SYSTEM_GPIO_EXTPHY_TABLE           \
        { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
        { GPIO_3,  GK_GPIO_I80_RDN     },

#define SYSTEM_GPIO_XREF_TABLE             \
        { GPIO_0 , GK_GPIO_SF_CS0           }, \
        { GPIO_1 , GK_GPIO_SF_CS1           }, \
        { GPIO_2 , GK_GPIO_PWM_3 /*GK_GPIO_PWM_3*/}, \
        { GPIO_4 , GK_GPIO_I80_WRN        }, \
        { GPIO_5 , GK_GPIO_I80_DCX          }, \
        { GPIO_6 , GK_GPIO_VD_VSYNC         }, \
        { GPIO_7 , GK_GPIO_VD_HSYNC         }, \
        { GPIO_8 , GK_GPIO_UNDEFINED        }, \
        { GPIO_9 , GK_GPIO_UNDEFINED        }, \
        { GPIO_10, GK_GPIO_UNDEFINED        }, \
        { GPIO_11, GK_GPIO_UNDEFINED        }, \
        { GPIO_12, GK_GPIO_UNDEFINED        }, \
        { GPIO_13, GK_GPIO_UNDEFINED        }, \
        SYSTEM_GPIO_SDCARD_TABLE\
        { GPIO_21, GK_GPIO_I80_DATA0         }, \
        { GPIO_23, GK_GPIO_I80_DATA1         }, \
        { GPIO_24, GK_GPIO_I80_DATA2         }, \
        { GPIO_25, GK_GPIO_UART0_RX         }, \
        { GPIO_26, GK_GPIO_UART0_TX         }, \
        { GPIO_27, GK_GPIO_I80_DATA3         }, \
        { GPIO_28, GK_GPIO_I80_DATA4         }, \
        { GPIO_29, GK_GPIO_SF_HOLD          }, \
        { GPIO_30, GK_GPIO_SF_WP            }, \
        { GPIO_31, GK_GPIO_UNDEFINED        }, \
        { GPIO_32, GK_GPIO_UNDEFINED        }, \
        { GPIO_33, GK_GPIO_UNDEFINED        }, \
        { GPIO_34, GK_GPIO_UNDEFINED        }, \
        { GPIO_35, GK_GPIO_UNDEFINED        }, \
        { GPIO_36, GK_GPIO_I80_DATA5         }, \
        { GPIO_37, GK_GPIO_I80_DATA6         }, \
        { GPIO_38, GK_GPIO_I80_DATA7         }, \
        { GPIO_39, GK_GPIO_VD_CLOCK         }, \
        { GPIO_40, GK_GPIO_I80_LCD_RST         }, \
        { GPIO_41, GK_GPIO_UNDEFINED        }, \
        { GPIO_42, GK_GPIO_UNDEFINED        }, \
        { GPIO_43, GK_GPIO_UNDEFINED        }, \
        { GPIO_44, GK_GPIO_SENSOR1_RESET    }, \
        { GPIO_45, GK_GPIO_UNDEFINED        }, \
        { GPIO_46, GK_GPIO_UNDEFINED        }, \
        { GPIO_47, GK_GPIO_UNDEFINED        }, \
        { GPIO_48, GK_GPIO_UNDEFINED        }, \
        { GPIO_49, GK_GPIO_UNDEFINED        }, \
        { GPIO_50, GK_GPIO_UNDEFINED        }, \
        { GPIO_51, GK_GPIO_UNDEFINED        }, \
        { GPIO_52, GK_GPIO_UNDEFINED        }, \
        { GPIO_53, GK_GPIO_UNDEFINED        }, \
        { GPIO_54, GK_GPIO_UNDEFINED        }, \
        { GPIO_55, GK_GPIO_UNDEFINED        }, \
        { GPIO_56, GK_GPIO_UNDEFINED        }, \
        { GPIO_57, GK_GPIO_UNDEFINED        }, \
        { GPIO_58, GK_GPIO_UNDEFINED        }, \
        { GPIO_59, GK_GPIO_UNDEFINED        }, \
        { GPIO_60, GK_GPIO_UNDEFINED        }, \
        { GPIO_61, GK_GPIO_I2C0_DATA        }, \
        { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
        { GPIO_63, GK_GPIO_I2C0_CLK         }, \
        { GPIO_64, GK_GPIO_UNDEFINED        }, \
        { GPIO_65, GK_GPIO_UNDEFINED        }, \
        { GPIO_66, GK_GPIO_UNDEFINED        }, \
        { GPIO_67, GK_GPIO_UNDEFINED        }, \
        { GPIO_68, GK_GPIO_UNDEFINED        }, \
        { GPIO_69, GK_GPIO_UNDEFINED        }, \
        { GPIO_70, GK_GPIO_UNDEFINED        }, \
        { GPIO_71, GK_GPIO_UNDEFINED        }, \
        { GPIO_72, GK_GPIO_UNDEFINED        }, \
        { GPIO_73, GK_GPIO_UNDEFINED        }, \
        { GPIO_74, GK_GPIO_UNDEFINED        }, \
        { GPIO_75, GK_GPIO_UNDEFINED        }, \
        { GPIO_76, GK_GPIO_UNDEFINED        }, \
        { GPIO_77, GK_GPIO_UNDEFINED        }, \
        { GPIO_78, GK_GPIO_UNDEFINED        }, \
        { GPIO_79, GK_GPIO_UNDEFINED        },


#if CONFIG_SYSTEM_VOUT == 2   //RGB 8bit
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
        { GPIO_14, GK_GPIO_SDIO0_WP_N       }, \
        { GPIO_15, GK_GPIO_SDIO0_DATA_1     }, \
        { GPIO_16, GK_GPIO_SDIO0_DATA_0     }, \
        { GPIO_17, GK_GPIO_SDIO0_CMD        }, \
        { GPIO_18, GK_GPIO_SDIO0_DATA_3     }, \
        { GPIO_19, GK_GPIO_SDIO0_DATA_2     }, \
        { GPIO_20, GK_GPIO_SDIO0_CD_N       }, \
        { GPIO_22, GK_GPIO_SDIO0_CLK        },
#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
        { GPIO_14, GK_GPIO_UNDEFINED        }, \
        { GPIO_15, GK_GPIO_UNDEFINED        }, \
        { GPIO_16, GK_GPIO_UNDEFINED        }, \
        { GPIO_17, GK_GPIO_UNDEFINED        }, \
        { GPIO_18, GK_GPIO_UNDEFINED        }, \
        { GPIO_19, GK_GPIO_UNDEFINED        }, \
        { GPIO_20, GK_GPIO_UNDEFINED        }, \
        { GPIO_22, GK_GPIO_UNDEFINED        },
#endif/* SYSTEM_GPIO_SDCARD_TABLE */

#define SYSTEM_GPIO_EXTPHY_TABLE           \
        { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
        { GPIO_3,  GK_GPIO_PHY_LINK_LED     },

#define SYSTEM_GPIO_XREF_TABLE             \
        { GPIO_0 , GK_GPIO_SF_CS0           }, \
        { GPIO_1 , GK_GPIO_SF_CS1           }, \
        { GPIO_2 , GK_GPIO_PWM_3 /*GK_GPIO_PWM_3*/}, \
        { GPIO_4 , GK_GPIO_UNDEFINED        }, \
        { GPIO_5 , GK_GPIO_VD_HVLD          }, \
        { GPIO_6 , GK_GPIO_VD_VSYNC         }, \
        { GPIO_7 , GK_GPIO_VD_HSYNC         }, \
        { GPIO_8 , GK_GPIO_UNDEFINED        }, \
        { GPIO_9 , GK_GPIO_UNDEFINED        }, \
        { GPIO_10, GK_GPIO_UNDEFINED        }, \
        { GPIO_11, GK_GPIO_UNDEFINED        }, \
        { GPIO_12, GK_GPIO_UNDEFINED        }, \
        { GPIO_13, GK_GPIO_UNDEFINED        }, \
        SYSTEM_GPIO_SDCARD_TABLE\
        { GPIO_21, GK_GPIO_VD_DATA7         }, \
        { GPIO_23, GK_GPIO_VD_DATA6         }, \
        { GPIO_24, GK_GPIO_VD_DATA5         }, \
        { GPIO_25, GK_GPIO_UART0_RX         }, \
        { GPIO_26, GK_GPIO_UART0_TX         }, \
        { GPIO_27, GK_GPIO_VD_DATA4         }, \
        { GPIO_28, GK_GPIO_VD_DATA3         }, \
        { GPIO_29, GK_GPIO_SF_HOLD          }, \
        { GPIO_30, GK_GPIO_SF_WP            }, \
        { GPIO_31, GK_GPIO_UNDEFINED        }, \
        { GPIO_32, GK_GPIO_UNDEFINED        }, \
        { GPIO_33, GK_GPIO_UNDEFINED        }, \
        { GPIO_34, GK_GPIO_UNDEFINED        }, \
        { GPIO_35, GK_GPIO_UNDEFINED        }, \
        { GPIO_36, GK_GPIO_VD_DATA2         }, \
        { GPIO_37, GK_GPIO_VD_DATA1         }, \
        { GPIO_38, GK_GPIO_VD_DATA0         }, \
        { GPIO_39, GK_GPIO_VD_CLOCK         }, \
        { GPIO_40, GK_GPIO_OUTPUT_1         }, \
        { GPIO_41, GK_GPIO_UNDEFINED        }, \
        { GPIO_42, GK_GPIO_UNDEFINED        }, \
        { GPIO_43, GK_GPIO_UNDEFINED        }, \
        { GPIO_44, GK_GPIO_SENSOR1_RESET    }, \
        { GPIO_45, GK_GPIO_UNDEFINED        }, \
        { GPIO_46, GK_GPIO_UNDEFINED        }, \
        { GPIO_47, GK_GPIO_UNDEFINED        }, \
        { GPIO_48, GK_GPIO_UNDEFINED        }, \
        { GPIO_49, GK_GPIO_UNDEFINED        }, \
        { GPIO_50, GK_GPIO_UNDEFINED        }, \
        { GPIO_51, GK_GPIO_UNDEFINED        }, \
        { GPIO_52, GK_GPIO_UNDEFINED        }, \
        { GPIO_53, GK_GPIO_UNDEFINED        }, \
        { GPIO_54, GK_GPIO_UNDEFINED        }, \
        { GPIO_55, GK_GPIO_UNDEFINED        }, \
        { GPIO_56, GK_GPIO_UNDEFINED        }, \
        { GPIO_57, GK_GPIO_UNDEFINED        }, \
        { GPIO_58, GK_GPIO_UNDEFINED        }, \
        { GPIO_59, GK_GPIO_UNDEFINED        }, \
        { GPIO_60, GK_GPIO_UNDEFINED        }, \
        { GPIO_61, GK_GPIO_I2C0_DATA        }, \
        { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
        { GPIO_63, GK_GPIO_I2C0_CLK         }, \
        { GPIO_64, GK_GPIO_UNDEFINED        }, \
        { GPIO_65, GK_GPIO_UNDEFINED        }, \
        { GPIO_66, GK_GPIO_UNDEFINED        }, \
        { GPIO_67, GK_GPIO_UNDEFINED        }, \
        { GPIO_68, GK_GPIO_UNDEFINED        }, \
        { GPIO_69, GK_GPIO_UNDEFINED        }, \
        { GPIO_70, GK_GPIO_UNDEFINED        }, \
        { GPIO_71, GK_GPIO_UNDEFINED        }, \
        { GPIO_72, GK_GPIO_UNDEFINED        }, \
        { GPIO_73, GK_GPIO_UNDEFINED        }, \
        { GPIO_74, GK_GPIO_UNDEFINED        }, \
        { GPIO_75, GK_GPIO_UNDEFINED        }, \
        { GPIO_76, GK_GPIO_UNDEFINED        }, \
        { GPIO_77, GK_GPIO_UNDEFINED        }, \
        { GPIO_78, GK_GPIO_UNDEFINED        }, \
        { GPIO_79, GK_GPIO_UNDEFINED        },
#elif CONFIG_SYSTEM_VOUT == 3   //RGB 24bit
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_4 , GK_GPIO_SDIO1_WP_N       }, \
    { GPIO_5 , GK_GPIO_SDIO1_DATA_1     }, \
    { GPIO_6 , GK_GPIO_SDIO1_DATA_0     }, \
    { GPIO_7 , GK_GPIO_SDIO1_CLK        }, \
    { GPIO_8 , GK_GPIO_SDIO1_CMD        }, \
    { GPIO_9 , GK_GPIO_SDIO1_DATA_3     }, \
    { GPIO_10, GK_GPIO_SDIO1_DATA_2     }, \
    { GPIO_11, GK_GPIO_SDIO1_CD_N       }, \
    { GPIO_14, GK_GPIO_SDIO0_WP_N       }, \
    { GPIO_15, GK_GPIO_SDIO0_DATA_1     }, \
    { GPIO_16, GK_GPIO_SDIO0_DATA_0     }, \
    { GPIO_17, GK_GPIO_SDIO0_CMD        }, \
    { GPIO_18, GK_GPIO_SDIO0_DATA_3     }, \
    { GPIO_19, GK_GPIO_SDIO0_DATA_2     }, \
    { GPIO_20, GK_GPIO_SDIO0_CD_N       }, \
    { GPIO_22, GK_GPIO_SDIO0_CLK        },

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
    { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
    { GPIO_6 , GK_GPIO_UNDEFINED        }, \
    { GPIO_7 , GK_GPIO_UNDEFINED        }, \
    { GPIO_8 , GK_GPIO_UNDEFINED        }, \
    { GPIO_9 , GK_GPIO_UNDEFINED        }, \
    { GPIO_10, GK_GPIO_UNDEFINED        }, \
    { GPIO_11, GK_GPIO_UNDEFINED        }, \
    { GPIO_14, GK_GPIO_UNDEFINED        }, \
    { GPIO_15, GK_GPIO_UNDEFINED        }, \
    { GPIO_16, GK_GPIO_UNDEFINED        }, \
    { GPIO_17, GK_GPIO_UNDEFINED        }, \
    { GPIO_18, GK_GPIO_UNDEFINED        }, \
    { GPIO_19, GK_GPIO_UNDEFINED        }, \
    { GPIO_20, GK_GPIO_UNDEFINED        }, \
    { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
    { GPIO_2 , GK_GPIO_UNDEFINED        }, \
    { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
    { GPIO_2,  GK_GPIO_UNDEFINED     }, \
    { GPIO_3,  GK_GPIO_UNDEFINED     },

#define SYSTEM_GPIO_XREF_TABLE             \
    { GPIO_0, GK_GPIO_SF_CS0           }, \
    { GPIO_1, GK_GPIO_SF_CS1       }, \
    { GPIO_2, GK_GPIO_PWM_3}, \
    { GPIO_3, GK_GPIO_UNDEFINED        }, \
    { GPIO_4, GK_GPIO_UNDEFINED        }, \
    { GPIO_5, GK_GPIO_VD_HVLD        }, \
    { GPIO_6, GK_GPIO_VD_VSYNC        }, \
    { GPIO_7, GK_GPIO_VD_HSYNC        }, \
    { GPIO_8, GK_GPIO_VD_DATA7        }, \
    { GPIO_9, GK_GPIO_VD_DATA6        }, \
    { GPIO_10, GK_GPIO_VD_DATA5        }, \
    { GPIO_11, GK_GPIO_VD_DATA4        }, \
    { GPIO_12, GK_GPIO_VD_DATA3        }, \
    { GPIO_13, GK_GPIO_VD_DATA2        }, \
    { GPIO_14, GK_GPIO_VD_DATA1        }, \
    { GPIO_15, GK_GPIO_VD_DATA0        }, \
    { GPIO_16, GK_GPIO_VD_DATA15        }, \
    { GPIO_17, GK_GPIO_VD_DATA13        }, \
    { GPIO_18, GK_GPIO_VD_DATA12        }, \
    { GPIO_19, GK_GPIO_VD_DATA11        }, \
    { GPIO_20, GK_GPIO_VD_DATA10        }, \
    { GPIO_21, GK_GPIO_VD_DATA23        }, \
    { GPIO_22, GK_GPIO_VD_DATA14        }, \
    { GPIO_23, GK_GPIO_VD_DATA22        }, \
    { GPIO_24, GK_GPIO_VD_DATA21        }, \
    { GPIO_25, GK_GPIO_UART0_RX         }, \
    { GPIO_26, GK_GPIO_UART0_TX         }, \
    { GPIO_27, GK_GPIO_VD_DATA20        }, \
    { GPIO_28, GK_GPIO_VD_DATA19        }, \
    { GPIO_29, GK_GPIO_UNDEFINED        }, \
    { GPIO_30, GK_GPIO_UNDEFINED        }, \
    { GPIO_31, GK_GPIO_UNDEFINED        }, \
    { GPIO_32, GK_GPIO_UNDEFINED        }, \
    { GPIO_33, GK_GPIO_UNDEFINED        }, \
    { GPIO_34, GK_GPIO_UNDEFINED        }, \
    { GPIO_35, GK_GPIO_UNDEFINED        }, \
    { GPIO_36, GK_GPIO_VD_DATA18        }, \
    { GPIO_37, GK_GPIO_VD_DATA17          }, \
    { GPIO_38, GK_GPIO_VD_DATA16          }, \
    { GPIO_39, GK_GPIO_VD_CLOCK        }, \
    { GPIO_40, GK_GPIO_OUTPUT_1         }, \
    { GPIO_41, GK_GPIO_UNDEFINED        }, \
    { GPIO_42, GK_GPIO_UNDEFINED        }, \
    { GPIO_43, GK_GPIO_UNDEFINED        }, \
    { GPIO_44, GK_GPIO_SENSOR1_RESET    }, \
    { GPIO_45, GK_GPIO_UNDEFINED        }, \
    { GPIO_46, GK_GPIO_UNDEFINED        }, \
    { GPIO_47, GK_GPIO_UNDEFINED        }, \
    { GPIO_48, GK_GPIO_UNDEFINED        }, \
    { GPIO_49, GK_GPIO_UNDEFINED        }, \
    { GPIO_50, GK_GPIO_UNDEFINED        }, \
    { GPIO_51, GK_GPIO_UNDEFINED        }, \
    { GPIO_52, GK_GPIO_UNDEFINED        }, \
    { GPIO_53, GK_GPIO_UNDEFINED        }, \
    { GPIO_54, GK_GPIO_UNDEFINED        }, \
    { GPIO_55, GK_GPIO_UNDEFINED        }, \
    { GPIO_56, GK_GPIO_UNDEFINED        }, \
    { GPIO_57, GK_GPIO_UNDEFINED        }, \
    { GPIO_58, GK_GPIO_UNDEFINED        }, \
    { GPIO_59, GK_GPIO_UNDEFINED        }, \
    { GPIO_60, GK_GPIO_UNDEFINED        }, \
    { GPIO_61, GK_GPIO_I2C0_DATA        }, \
    { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
    { GPIO_63, GK_GPIO_I2C0_CLK         }, \
    { GPIO_64, GK_GPIO_UNDEFINED        }, \
    { GPIO_65, GK_GPIO_UNDEFINED        }, \
    { GPIO_66, GK_GPIO_UNDEFINED        }, \
    { GPIO_67, GK_GPIO_UNDEFINED        }, \
    { GPIO_68, GK_GPIO_UNDEFINED        }, \
    { GPIO_69, GK_GPIO_UNDEFINED        }, \
    { GPIO_70, GK_GPIO_UNDEFINED        }, \
    { GPIO_71, GK_GPIO_UNDEFINED        }, \
    { GPIO_72, GK_GPIO_UNDEFINED        }, \
    { GPIO_73, GK_GPIO_UNDEFINED        }, \
    { GPIO_74, GK_GPIO_UNDEFINED        }, \
    { GPIO_75, GK_GPIO_UNDEFINED        }, \
    { GPIO_76, GK_GPIO_UNDEFINED        }, \
    { GPIO_77, GK_GPIO_UNDEFINED        }, \
    { GPIO_78, GK_GPIO_UNDEFINED        }, \
    { GPIO_79, GK_GPIO_UNDEFINED        },


#elif CONFIG_SYSTEM_VOUT == 4   //4--RGB 16bit use to 24bit LCD data wire (OK)
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_4 , GK_GPIO_SDIO1_WP_N       }, \
    { GPIO_5 , GK_GPIO_SDIO1_DATA_1     }, \
    { GPIO_6 , GK_GPIO_SDIO1_DATA_0     }, \
    { GPIO_7 , GK_GPIO_SDIO1_CLK        }, \
    { GPIO_8 , GK_GPIO_SDIO1_CMD        }, \
    { GPIO_9 , GK_GPIO_SDIO1_DATA_3     }, \
    { GPIO_10, GK_GPIO_SDIO1_DATA_2     }, \
    { GPIO_11, GK_GPIO_SDIO1_CD_N       }, \
    { GPIO_14, GK_GPIO_SDIO0_WP_N       }, \
    { GPIO_15, GK_GPIO_SDIO0_DATA_1     }, \
    { GPIO_16, GK_GPIO_SDIO0_DATA_0     }, \
    { GPIO_17, GK_GPIO_SDIO0_CMD        }, \
    { GPIO_18, GK_GPIO_SDIO0_DATA_3     }, \
    { GPIO_19, GK_GPIO_SDIO0_DATA_2     }, \
    { GPIO_20, GK_GPIO_SDIO0_CD_N       }, \
    { GPIO_22, GK_GPIO_SDIO0_CLK        },

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
    { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
    { GPIO_6 , GK_GPIO_UNDEFINED        }, \
    { GPIO_7 , GK_GPIO_UNDEFINED        }, \
    { GPIO_8 , GK_GPIO_UNDEFINED        }, \
    { GPIO_9 , GK_GPIO_UNDEFINED        }, \
    { GPIO_10, GK_GPIO_UNDEFINED        }, \
    { GPIO_11, GK_GPIO_UNDEFINED        }, \
    { GPIO_14, GK_GPIO_UNDEFINED        }, \
    { GPIO_15, GK_GPIO_UNDEFINED        }, \
    { GPIO_16, GK_GPIO_UNDEFINED        }, \
    { GPIO_17, GK_GPIO_UNDEFINED        }, \
    { GPIO_18, GK_GPIO_UNDEFINED        }, \
    { GPIO_19, GK_GPIO_UNDEFINED        }, \
    { GPIO_20, GK_GPIO_UNDEFINED        }, \
    { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
    { GPIO_2 , GK_GPIO_UNDEFINED        }, \
    { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
    { GPIO_2,  GK_GPIO_UNDEFINED     }, \
    { GPIO_3,  GK_GPIO_UNDEFINED     },

#define SYSTEM_GPIO_XREF_TABLE             \
    { GPIO_0, GK_GPIO_SF_CS0           }, \
    { GPIO_1, GK_GPIO_SF_CS1       }, \
    { GPIO_2, GK_GPIO_PWM_3}, \
    { GPIO_3, GK_GPIO_UNDEFINED        }, \
    { GPIO_4, GK_GPIO_UNDEFINED        }, \
    { GPIO_5, GK_GPIO_VD_HVLD        }, \
    { GPIO_6, GK_GPIO_VD_VSYNC        }, \
    { GPIO_7, GK_GPIO_VD_HSYNC        }, \
    { GPIO_8, GK_GPIO_VD_DATA4        }, \
    { GPIO_9, GK_GPIO_VD_DATA3        }, \
    { GPIO_10, GK_GPIO_VD_DATA2        }, \
    { GPIO_11, GK_GPIO_VD_DATA1        }, \
    { GPIO_12, GK_GPIO_VD_DATA0        }, \
    { GPIO_13, GK_GPIO_UNDEFINED        }, \
    { GPIO_14, GK_GPIO_UNDEFINED        }, \
    { GPIO_15, GK_GPIO_UNDEFINED        }, \
    { GPIO_16, GK_GPIO_VD_DATA10        }, \
    { GPIO_17, GK_GPIO_VD_DATA8        }, \
    { GPIO_18, GK_GPIO_VD_DATA7        }, \
    { GPIO_19, GK_GPIO_VD_DATA6        }, \
    { GPIO_20, GK_GPIO_VD_DATA5        }, \
    { GPIO_21, GK_GPIO_VD_DATA15        }, \
    { GPIO_22, GK_GPIO_VD_DATA9        }, \
    { GPIO_23, GK_GPIO_VD_DATA14        }, \
    { GPIO_24, GK_GPIO_VD_DATA13        }, \
    { GPIO_25, GK_GPIO_UART0_RX         }, \
    { GPIO_26, GK_GPIO_UART0_TX         }, \
    { GPIO_27, GK_GPIO_VD_DATA12        }, \
    { GPIO_28, GK_GPIO_VD_DATA11        }, \
    { GPIO_29, GK_GPIO_UNDEFINED        }, \
    { GPIO_30, GK_GPIO_UNDEFINED        }, \
    { GPIO_31, GK_GPIO_UNDEFINED        }, \
    { GPIO_32, GK_GPIO_UNDEFINED        }, \
    { GPIO_33, GK_GPIO_UNDEFINED        }, \
    { GPIO_34, GK_GPIO_UNDEFINED        }, \
    { GPIO_35, GK_GPIO_UNDEFINED        }, \
    { GPIO_36, GK_GPIO_UNDEFINED        }, \
    { GPIO_37, GK_GPIO_UNDEFINED          }, \
    { GPIO_38, GK_GPIO_UNDEFINED          }, \
    { GPIO_39, GK_GPIO_VD_CLOCK        }, \
    { GPIO_40, GK_GPIO_OUTPUT_1         }, \
    { GPIO_41, GK_GPIO_UNDEFINED        }, \
    { GPIO_42, GK_GPIO_UNDEFINED        }, \
    { GPIO_43, GK_GPIO_UNDEFINED        }, \
    { GPIO_44, GK_GPIO_SENSOR1_RESET    }, \
    { GPIO_45, GK_GPIO_UNDEFINED        }, \
    { GPIO_46, GK_GPIO_UNDEFINED        }, \
    { GPIO_47, GK_GPIO_UNDEFINED        }, \
    { GPIO_48, GK_GPIO_UNDEFINED        }, \
    { GPIO_49, GK_GPIO_UNDEFINED        }, \
    { GPIO_50, GK_GPIO_UNDEFINED        }, \
    { GPIO_51, GK_GPIO_UNDEFINED        }, \
    { GPIO_52, GK_GPIO_UNDEFINED        }, \
    { GPIO_53, GK_GPIO_UNDEFINED        }, \
    { GPIO_54, GK_GPIO_UNDEFINED        }, \
    { GPIO_55, GK_GPIO_UNDEFINED        }, \
    { GPIO_56, GK_GPIO_UNDEFINED        }, \
    { GPIO_57, GK_GPIO_UNDEFINED        }, \
    { GPIO_58, GK_GPIO_UNDEFINED        }, \
    { GPIO_59, GK_GPIO_UNDEFINED        }, \
    { GPIO_60, GK_GPIO_UNDEFINED        }, \
    { GPIO_61, GK_GPIO_I2C0_DATA        }, \
    { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
    { GPIO_63, GK_GPIO_I2C0_CLK         }, \
    { GPIO_64, GK_GPIO_UNDEFINED        }, \
    { GPIO_65, GK_GPIO_UNDEFINED        }, \
    { GPIO_66, GK_GPIO_UNDEFINED        }, \
    { GPIO_67, GK_GPIO_UNDEFINED        }, \
    { GPIO_68, GK_GPIO_UNDEFINED        }, \
    { GPIO_69, GK_GPIO_UNDEFINED        }, \
    { GPIO_70, GK_GPIO_UNDEFINED        }, \
    { GPIO_71, GK_GPIO_UNDEFINED        }, \
    { GPIO_72, GK_GPIO_UNDEFINED        }, \
    { GPIO_73, GK_GPIO_UNDEFINED        }, \
    { GPIO_74, GK_GPIO_UNDEFINED        }, \
    { GPIO_75, GK_GPIO_UNDEFINED        }, \
    { GPIO_76, GK_GPIO_UNDEFINED        }, \
    { GPIO_77, GK_GPIO_UNDEFINED        }, \
    { GPIO_78, GK_GPIO_UNDEFINED        }, \
    { GPIO_79, GK_GPIO_UNDEFINED        },

#elif CONFIG_SYSTEM_VOUT == 5   //RGB 8bit with SPI0(undefined)
#define SYSTEM_GPIO_XREF_TABLE


#elif CONFIG_SYSTEM_VOUT == 6   //RGB 8bit with SPI1(undefined)
#define SYSTEM_GPIO_XREF_TABLE


#elif CONFIG_SYSTEM_VOUT == 0                            //CONFIG_SYSTEM_VOUT == 0    Close Vout

#if CONFIG_STSTEM_DUL_VIN == 1
#ifdef CONFIG_SYSTEM_USE_SDCARD

#define SYSTEM_GPIO_SDCARD_TABLE        \
        { GPIO_4 , GK_GPIO_SDIO1_WP_N       }, \
        { GPIO_5 , GK_GPIO_SDIO1_DATA_1     }, \
        { GPIO_6 , GK_GPIO_SDIO1_DATA_0     }, \
        { GPIO_7 , GK_GPIO_SDIO1_CLK        }, \
        { GPIO_8 , GK_GPIO_SDIO1_CMD        }, \
        { GPIO_9 , GK_GPIO_SDIO1_DATA_3     }, \
        { GPIO_10, GK_GPIO_SDIO1_DATA_2     }, \
        { GPIO_11, GK_GPIO_SDIO1_CD_N       }, \
        { GPIO_14, GK_GPIO_SDIO0_WP_N       }, \
        { GPIO_15, GK_GPIO_SDIO0_DATA_1     }, \
        { GPIO_16, GK_GPIO_SDIO0_DATA_0     }, \
        { GPIO_17, GK_GPIO_SDIO0_CMD        }, \
        { GPIO_18, GK_GPIO_SDIO0_DATA_3     }, \
        { GPIO_19, GK_GPIO_SDIO0_DATA_2     }, \
        { GPIO_20, GK_GPIO_SDIO0_CD_N       }, \
        { GPIO_22, GK_GPIO_SDIO0_CLK        },
#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
        { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
        { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
        { GPIO_6 , GK_GPIO_UNDEFINED        }, \
        { GPIO_7 , GK_GPIO_UNDEFINED        }, \
        { GPIO_8 , GK_GPIO_UNDEFINED        }, \
        { GPIO_9 , GK_GPIO_UNDEFINED        }, \
        { GPIO_10, GK_GPIO_UNDEFINED        }, \
        { GPIO_11, GK_GPIO_UNDEFINED        }, \
        { GPIO_14, GK_GPIO_UNDEFINED        }, \
        { GPIO_15, GK_GPIO_UNDEFINED        }, \
        { GPIO_16, GK_GPIO_UNDEFINED        }, \
        { GPIO_17, GK_GPIO_UNDEFINED        }, \
        { GPIO_18, GK_GPIO_UNDEFINED        }, \
        { GPIO_19, GK_GPIO_UNDEFINED        }, \
        { GPIO_20, GK_GPIO_UNDEFINED        }, \
        { GPIO_22, GK_GPIO_UNDEFINED        },
#endif/* SYSTEM_GPIO_SDCARD_TABLE */

#define SYSTEM_GPIO_EXTPHY_TABLE           \
        { GPIO_2 , GK_GPIO_UNDEFINED        }, \
        { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
        { GPIO_2,  GK_GPIO_PHY_DATA_LED     }, \
        { GPIO_3,  GK_GPIO_PHY_LINK_LED     },

#define SYSTEM_GPIO_XREF_TABLE             \
        { GPIO_0 , GK_GPIO_SF_CS0           }, \
        { GPIO_1 , GK_GPIO_SF_CS1           }, \
        SYSTEM_GPIO_SDCARD_TABLE\
        { GPIO_12, GK_GPIO_UNDEFINED        }, \
        { GPIO_13, GK_GPIO_UNDEFINED        }, \
        { GPIO_21, GK_GPIO_UNDEFINED        }, \
        { GPIO_23, GK_GPIO_UNDEFINED        }, \
        { GPIO_24, GK_GPIO_UNDEFINED        }, \
        { GPIO_25, GK_GPIO_UART0_RX         }, \
        { GPIO_26, GK_GPIO_UART0_TX         }, \
        { GPIO_27, GK_GPIO_UNDEFINED        }, \
        { GPIO_28, GK_GPIO_UNDEFINED        }, \
        { GPIO_29, GK_GPIO_SF_HOLD          }, \
        { GPIO_30, GK_GPIO_SF_WP            }, \
        { GPIO_31, GK_GPIO_UNDEFINED        }, \
        { GPIO_32, GK_GPIO_UNDEFINED        }, \
        { GPIO_33, GK_GPIO_UNDEFINED        }, \
        { GPIO_34, GK_GPIO_UNDEFINED        }, \
        { GPIO_35, GK_GPIO_UNDEFINED        }, \
        { GPIO_36, GK_GPIO_SPI0_SCLK        }, \
        { GPIO_37, GK_GPIO_SPI0_SO          }, \
        { GPIO_38, GK_GPIO_SPI0_SI          }, \
        { GPIO_39, GK_GPIO_I2C1_DATA        }, \
        { GPIO_40, GK_GPIO_I2C1_CLK         }, \
        { GPIO_41, GK_GPIO_I2C0_DATA        }, \
        { GPIO_42, GK_GPIO_UNDEFINED        }, \
        { GPIO_43, GK_GPIO_I2C0_CLK         }, \
        { GPIO_44, GK_GPIO_SENSOR1_RESET    }, \
        { GPIO_45, GK_GPIO_UNDEFINED        }, \
        { GPIO_46, GK_GPIO_UNDEFINED        }, \
        { GPIO_47, GK_GPIO_UNDEFINED        }, \
        { GPIO_48, GK_GPIO_UNDEFINED        }, \
        { GPIO_49, GK_GPIO_UNDEFINED        }, \
        { GPIO_50, GK_GPIO_UNDEFINED        }, \
        { GPIO_51, GK_GPIO_UNDEFINED        }, \
        { GPIO_52, GK_GPIO_UNDEFINED        }, \
        { GPIO_53, GK_GPIO_UNDEFINED        }, \
        { GPIO_54, GK_GPIO_UNDEFINED        }, \
        { GPIO_55, GK_GPIO_UNDEFINED        }, \
        { GPIO_56, GK_GPIO_UNDEFINED        }, \
        { GPIO_57, GK_GPIO_UNDEFINED        }, \
        { GPIO_58, GK_GPIO_UNDEFINED        }, \
        { GPIO_59, GK_GPIO_UNDEFINED        }, \
        { GPIO_60, GK_GPIO_UNDEFINED        }, \
        { GPIO_61, GK_GPIO_UNDEFINED        }, \
        { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
        { GPIO_63, GK_GPIO_UNDEFINED        }, \
        { GPIO_64, GK_GPIO_UNDEFINED        }, \
        { GPIO_65, GK_GPIO_UNDEFINED        }, \
        { GPIO_66, GK_GPIO_UNDEFINED        }, \
        { GPIO_67, GK_GPIO_UNDEFINED        }, \
        { GPIO_68, GK_GPIO_UNDEFINED        }, \
        { GPIO_69, GK_GPIO_UNDEFINED        }, \
        { GPIO_70, GK_GPIO_UNDEFINED        }, \
        { GPIO_71, GK_GPIO_UNDEFINED        }, \
        { GPIO_72, GK_GPIO_UNDEFINED        }, \
        { GPIO_73, GK_GPIO_UNDEFINED        }, \
        { GPIO_74, GK_GPIO_UNDEFINED        }, \
        { GPIO_75, GK_GPIO_UNDEFINED        }, \
        { GPIO_76, GK_GPIO_UNDEFINED        }, \
        { GPIO_77, GK_GPIO_UNDEFINED        }, \
        { GPIO_78, GK_GPIO_UNDEFINED        }, \
        { GPIO_79, GK_GPIO_UNDEFINED        },
#else  //CONFIG_STSTEM_DUL_VIN == 0
#ifdef CONFIG_SYSTEM_USE_SDCARD

#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_4 , GK_GPIO_SDIO1_WP_N       }, \
            { GPIO_5 , GK_GPIO_SDIO1_DATA_1     }, \
            { GPIO_6 , GK_GPIO_SDIO1_DATA_0     }, \
            { GPIO_7 , GK_GPIO_SDIO1_CLK        }, \
            { GPIO_8 , GK_GPIO_SDIO1_CMD        }, \
            { GPIO_9 , GK_GPIO_SDIO1_DATA_3     }, \
            { GPIO_10, GK_GPIO_SDIO1_DATA_2     }, \
            { GPIO_11, GK_GPIO_SDIO1_CD_N       }, \
            { GPIO_14, GK_GPIO_SDIO0_WP_N       }, \
            { GPIO_15, GK_GPIO_SDIO0_DATA_1     }, \
            { GPIO_16, GK_GPIO_SDIO0_DATA_0     }, \
            { GPIO_17, GK_GPIO_SDIO0_CMD        }, \
            { GPIO_18, GK_GPIO_SDIO0_DATA_3     }, \
            { GPIO_19, GK_GPIO_SDIO0_DATA_2     }, \
            { GPIO_20, GK_GPIO_SDIO0_CD_N       }, \
            { GPIO_22, GK_GPIO_SDIO0_CLK        },
#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
            { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
            { GPIO_6 , GK_GPIO_UNDEFINED        }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
            { GPIO_9 , GK_GPIO_UNDEFINED        }, \
            { GPIO_10, GK_GPIO_UNDEFINED        }, \
            { GPIO_11, GK_GPIO_UNDEFINED        }, \
            { GPIO_14, GK_GPIO_UNDEFINED        }, \
            { GPIO_15, GK_GPIO_UNDEFINED        }, \
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_UNDEFINED        }, \
            { GPIO_20, GK_GPIO_UNDEFINED        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },
#endif/* SYSTEM_GPIO_SDCARD_TABLE */

#define SYSTEM_GPIO_EXTPHY_TABLE           \
            { GPIO_2 , GK_GPIO_UNDEFINED        }, \
            { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
            { GPIO_2,  GK_GPIO_HDMI_BT_RESET     }, \
            { GPIO_3,  GK_GPIO_PHY_LINK_LED     },

#define SYSTEM_GPIO_XREF_TABLE             \
            { GPIO_0 , GK_GPIO_SF_CS0           }, \
            { GPIO_1 , GK_GPIO_SF_CS1           }, \
            SYSTEM_GPIO_SDCARD_TABLE\
            { GPIO_12, GK_GPIO_UNDEFINED        }, \
            { GPIO_13, GK_GPIO_UNDEFINED        }, \
            { GPIO_21, GK_GPIO_UNDEFINED        }, \
            { GPIO_23, GK_GPIO_UNDEFINED        }, \
            { GPIO_24, GK_GPIO_UNDEFINED        }, \
            { GPIO_25, GK_GPIO_UART0_RX         }, \
            { GPIO_26, GK_GPIO_UART0_TX         }, \
            { GPIO_27, GK_GPIO_UNDEFINED        }, \
            { GPIO_28, GK_GPIO_UNDEFINED        }, \
            { GPIO_29, GK_GPIO_SF_HOLD          }, \
            { GPIO_30, GK_GPIO_SF_WP            }, \
            { GPIO_31, GK_GPIO_UNDEFINED        }, \
            { GPIO_32, GK_GPIO_UNDEFINED        }, \
            { GPIO_33, GK_GPIO_UNDEFINED        }, \
            { GPIO_34, GK_GPIO_UNDEFINED        }, \
            { GPIO_35, GK_GPIO_UNDEFINED        }, \
            { GPIO_36, GK_GPIO_SPI0_SCLK        }, \
            { GPIO_37, GK_GPIO_SPI0_SO          }, \
            { GPIO_38, GK_GPIO_SPI0_SI          }, \
            { GPIO_39, GK_GPIO_I2C1_DATA        }, \
            { GPIO_40, GK_GPIO_I2C1_CLK         }, \
            { GPIO_41, GK_GPIO_UNDEFINED        }, \
            { GPIO_42, GK_GPIO_UNDEFINED        }, \
            { GPIO_43, GK_GPIO_UNDEFINED        }, \
            { GPIO_44, GK_GPIO_SENSOR1_RESET    }, \
            { GPIO_45, GK_GPIO_UNDEFINED        }, \
            { GPIO_46, GK_GPIO_UNDEFINED        }, \
            { GPIO_47, GK_GPIO_UNDEFINED        }, \
            { GPIO_48, GK_GPIO_UNDEFINED        }, \
            { GPIO_49, GK_GPIO_UNDEFINED        }, \
            { GPIO_50, GK_GPIO_UNDEFINED        }, \
            { GPIO_51, GK_GPIO_UNDEFINED        }, \
            { GPIO_52, GK_GPIO_UNDEFINED        }, \
            { GPIO_53, GK_GPIO_UNDEFINED        }, \
            { GPIO_54, GK_GPIO_UNDEFINED        }, \
            { GPIO_55, GK_GPIO_UNDEFINED        }, \
            { GPIO_56, GK_GPIO_UNDEFINED        }, \
            { GPIO_57, GK_GPIO_UNDEFINED        }, \
            { GPIO_58, GK_GPIO_UNDEFINED        }, \
            { GPIO_59, GK_GPIO_UNDEFINED        }, \
            { GPIO_60, GK_GPIO_UNDEFINED        }, \
            { GPIO_61, GK_GPIO_I2C0_DATA        }, \
            { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
            { GPIO_63, GK_GPIO_I2C0_CLK         }, \
            { GPIO_64, GK_GPIO_UNDEFINED        }, \
            { GPIO_65, GK_GPIO_UNDEFINED        }, \
            { GPIO_66, GK_GPIO_UNDEFINED        }, \
            { GPIO_67, GK_GPIO_UNDEFINED        }, \
            { GPIO_68, GK_GPIO_UNDEFINED        }, \
            { GPIO_69, GK_GPIO_UNDEFINED        }, \
            { GPIO_70, GK_GPIO_UNDEFINED        }, \
            { GPIO_71, GK_GPIO_UNDEFINED        }, \
            { GPIO_72, GK_GPIO_UNDEFINED        }, \
            { GPIO_73, GK_GPIO_UNDEFINED        }, \
            { GPIO_74, GK_GPIO_UNDEFINED        }, \
            { GPIO_75, GK_GPIO_UNDEFINED        }, \
            { GPIO_76, GK_GPIO_UNDEFINED        }, \
            { GPIO_77, GK_GPIO_UNDEFINED        }, \
            { GPIO_78, GK_GPIO_UNDEFINED        }, \
            { GPIO_79, GK_GPIO_UNDEFINED        },

#endif /*CONFIG_STSTEM_DUL_VIN*/


#endif /* CONFIG_MACH_GK_T26_R25_B2 */
#endif /* CONFIG_MACH_GK_T26_R25_B2 */


#if defined(CONFIG_MACH_GK_T26_R25_C)
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_4 , GK_GPIO_SDIO1_WP_N       }, \
    { GPIO_5 , GK_GPIO_SDIO1_DATA_1     }, \
    { GPIO_6 , GK_GPIO_SDIO1_DATA_0     }, \
    { GPIO_7 , GK_GPIO_SDIO1_CLK        }, \
    { GPIO_8 , GK_GPIO_SDIO1_CMD        }, \
    { GPIO_9 , GK_GPIO_SDIO1_DATA_3     }, \
    { GPIO_10, GK_GPIO_SDIO1_DATA_2     }, \
    { GPIO_11, GK_GPIO_SDIO1_CD_N       }, \
    { GPIO_15, GK_GPIO_SDIO0_DATA_1     }, \
    { GPIO_16, GK_GPIO_SDIO0_DATA_0     }, \
    { GPIO_17, GK_GPIO_SDIO0_CMD        }, \
    { GPIO_18, GK_GPIO_SDIO0_DATA_3     }, \
    { GPIO_19, GK_GPIO_SDIO0_DATA_2     }, \
    { GPIO_20, GK_GPIO_SDIO0_CD_N       }, \
    { GPIO_22, GK_GPIO_SDIO0_CLK        },

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
    { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
    { GPIO_6 , GK_GPIO_UNDEFINED        }, \
    { GPIO_7 , GK_GPIO_UNDEFINED        }, \
    { GPIO_8 , GK_GPIO_UNDEFINED        }, \
    { GPIO_9 , GK_GPIO_UNDEFINED        }, \
    { GPIO_10, GK_GPIO_UNDEFINED        }, \
    { GPIO_11, GK_GPIO_UNDEFINED        }, \
    { GPIO_14, GK_GPIO_UNDEFINED        }, \
    { GPIO_15, GK_GPIO_UNDEFINED        }, \
    { GPIO_16, GK_GPIO_UNDEFINED        }, \
    { GPIO_17, GK_GPIO_UNDEFINED        }, \
    { GPIO_18, GK_GPIO_UNDEFINED        }, \
    { GPIO_19, GK_GPIO_UNDEFINED        }, \
    { GPIO_20, GK_GPIO_UNDEFINED        }, \
    { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
    { GPIO_2 , GK_GPIO_UNDEFINED        }, \
    { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
    { GPIO_2,  GK_GPIO_PHY_DATA_LED     }, \
    { GPIO_3,  GK_GPIO_PHY_LINK_LED     }, \
    { GPIO_58 , GK_GPIO_PHY_SPEED_LED},/*C*/

#define SYSTEM_GPIO_XREF_TABLE             \
    { GPIO_0 , GK_GPIO_SF_CS0           }, \
    { GPIO_1 , GK_GPIO_SF_CS1       }, \
    SYSTEM_GPIO_SDCARD_TABLE\
    { GPIO_12, GK_GPIO_UNDEFINED        }, \
    { GPIO_13, GK_GPIO_UNDEFINED        }, \
    { GPIO_14, GK_GPIO_IR_DETECT        }, \
    { GPIO_21, GK_GPIO_UNDEFINED        }, \
    { GPIO_23, GK_GPIO_UNDEFINED        }, \
    { GPIO_24, GK_GPIO_UNDEFINED        }, \
    { GPIO_25, GK_GPIO_UART0_RX         }, \
    { GPIO_26, GK_GPIO_UART0_TX         }, \
    { GPIO_27, GK_GPIO_IR_CUT2        }, \
    { GPIO_28, GK_GPIO_IR_CUT1        }, \
    { GPIO_29, GK_GPIO_SF_HOLD        }, \
    { GPIO_30, GK_GPIO_SF_WP        }, \
    { GPIO_31, GK_GPIO_UNDEFINED        }, \
    { GPIO_32, GK_GPIO_UNDEFINED        }, \
    { GPIO_33, GK_GPIO_UNDEFINED        }, \
    { GPIO_34, GK_GPIO_UNDEFINED        }, \
    { GPIO_35, GK_GPIO_UNDEFINED        }, \
    { GPIO_36, GK_GPIO_SPI0_SCLK        }, \
    { GPIO_37, GK_GPIO_SPI0_SO          }, \
    { GPIO_38, GK_GPIO_SPI0_SI          }, \
    { GPIO_39, GK_GPIO_UNDEFINED        }, \
    { GPIO_40, GK_GPIO_UNDEFINED         }, \
    { GPIO_41, GK_GPIO_UNDEFINED        }, \
    { GPIO_42, GK_GPIO_UNDEFINED        }, \
    { GPIO_43, GK_GPIO_UNDEFINED        }, \
    { GPIO_44, GK_GPIO_SENSOR1_RESET    }, \
    { GPIO_45, GK_GPIO_UNDEFINED        }, \
    { GPIO_46, GK_GPIO_UNDEFINED        }, \
    { GPIO_47, GK_GPIO_UNDEFINED        }, \
    { GPIO_48, GK_GPIO_UNDEFINED        }, \
    { GPIO_49, GK_GPIO_UNDEFINED        }, \
    { GPIO_50, GK_GPIO_UNDEFINED        }, \
    { GPIO_51, GK_GPIO_UNDEFINED        }, \
    { GPIO_52, GK_GPIO_UNDEFINED        }, \
    { GPIO_53, GK_GPIO_UNDEFINED        }, \
    { GPIO_54, GK_GPIO_UNDEFINED        }, \
    { GPIO_55, GK_GPIO_UNDEFINED        }, \
    { GPIO_56, GK_GPIO_UNDEFINED        }, \
    { GPIO_57, GK_GPIO_UNDEFINED        }, \
    { GPIO_59, GK_GPIO_UNDEFINED        }, \
    { GPIO_60, GK_GPIO_UNDEFINED        }, \
    { GPIO_61, GK_GPIO_I2C0_DATA        }, \
    { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
    { GPIO_63, GK_GPIO_I2C0_CLK         }, \
    { GPIO_64, GK_GPIO_UNDEFINED        }, \
    { GPIO_65, GK_GPIO_UNDEFINED        }, \
    { GPIO_66, GK_GPIO_UNDEFINED        }, \
    { GPIO_67, GK_GPIO_UNDEFINED        }, \
    { GPIO_68, GK_GPIO_UNDEFINED        }, \
    { GPIO_69, GK_GPIO_UNDEFINED        }, \
    { GPIO_70, GK_GPIO_UNDEFINED        }, \
    { GPIO_71, GK_GPIO_UNDEFINED        }, \
    { GPIO_72, GK_GPIO_UNDEFINED        }, \
    { GPIO_73, GK_GPIO_UNDEFINED        }, \
    { GPIO_74, GK_GPIO_UNDEFINED        }, \
    { GPIO_75, GK_GPIO_UNDEFINED        }, \
    { GPIO_76, GK_GPIO_UNDEFINED        }, \
    { GPIO_77, GK_GPIO_UNDEFINED        }, \
    { GPIO_78, GK_GPIO_UNDEFINED        }, \
    { GPIO_79, GK_GPIO_UNDEFINED        },
#endif /* CONFIG_MACH_GK_T26_R25_C */

#if defined(CONFIG_MACH_GK_T26_R25_D)
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_4 , GK_GPIO_SDIO1_WP_N       }, \
    { GPIO_5 , GK_GPIO_SDIO1_DATA_1     }, \
    { GPIO_6 , GK_GPIO_SDIO1_DATA_0     }, \
    { GPIO_7 , GK_GPIO_SDIO1_CLK        }, \
    { GPIO_8 , GK_GPIO_SDIO1_CMD        }, \
    { GPIO_9 , GK_GPIO_SDIO1_DATA_3     }, \
    { GPIO_10, GK_GPIO_SDIO1_DATA_2     }, \
    { GPIO_11, GK_GPIO_SDIO1_CD_N       }, \
    { GPIO_15, GK_GPIO_SDIO0_DATA_1     }, \
    { GPIO_16, GK_GPIO_SDIO0_DATA_0     }, \
    { GPIO_17, GK_GPIO_SDIO0_CMD        }, \
    { GPIO_18, GK_GPIO_SDIO0_DATA_3     }, \
    { GPIO_19, GK_GPIO_SDIO0_DATA_2     }, \
    { GPIO_20, GK_GPIO_SDIO0_CD_N       }, \
    { GPIO_22, GK_GPIO_SDIO0_CLK        },

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
    { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
    { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
    { GPIO_6 , GK_GPIO_UNDEFINED        }, \
    { GPIO_7 , GK_GPIO_UNDEFINED        }, \
    { GPIO_8 , GK_GPIO_UNDEFINED        }, \
    { GPIO_9 , GK_GPIO_UNDEFINED        }, \
    { GPIO_10, GK_GPIO_UNDEFINED        }, \
    { GPIO_11, GK_GPIO_UNDEFINED        }, \
    { GPIO_14, GK_GPIO_UNDEFINED        }, \
    { GPIO_15, GK_GPIO_UNDEFINED        }, \
    { GPIO_16, GK_GPIO_UNDEFINED        }, \
    { GPIO_17, GK_GPIO_UNDEFINED        }, \
    { GPIO_18, GK_GPIO_UNDEFINED        }, \
    { GPIO_19, GK_GPIO_UNDEFINED        }, \
    { GPIO_20, GK_GPIO_UNDEFINED        }, \
    { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
    { GPIO_2 , GK_GPIO_UNDEFINED        }, \
    { GPIO_3 , GK_GPIO_UNDEFINED        },\
    { GPIO_36 , GK_GPIO_ENET_PHY_RXD_0},/*D*/

#define SYSTEM_GPIO_INTPHY_TABLE           \
    { GPIO_2,  GK_GPIO_PHY_DATA_LED     }, \
    { GPIO_3,  GK_GPIO_PHY_LINK_LED     }, \
    { GPIO_36 , GK_GPIO_PHY_SPEED_LED},/*D*/

#define SYSTEM_GPIO_XREF_TABLE             \
    { GPIO_0 , GK_GPIO_SF_CS0           }, \
    { GPIO_1 , GK_GPIO_SF_CS1       }, \
    SYSTEM_GPIO_SDCARD_TABLE\
    { GPIO_12, GK_GPIO_UNDEFINED        }, \
    { GPIO_13, GK_GPIO_UNDEFINED        }, \
    { GPIO_14, GK_GPIO_UNDEFINED        }, \
    { GPIO_21, GK_GPIO_UNDEFINED        }, \
    { GPIO_23, GK_GPIO_UNDEFINED        }, \
    { GPIO_24, GK_GPIO_UNDEFINED        }, \
    { GPIO_25, GK_GPIO_UART0_RX         }, \
    { GPIO_26, GK_GPIO_UART0_TX         }, \
    { GPIO_27, GK_GPIO_UNDEFINED        }, \
    { GPIO_28, GK_GPIO_IR_DETECT        }, \
    { GPIO_29, GK_GPIO_SF_HOLD        }, \
    { GPIO_30, GK_GPIO_SF_WP        }, \
    { GPIO_31, GK_GPIO_UNDEFINED        }, \
    { GPIO_32, GK_GPIO_UNDEFINED        }, \
    { GPIO_33, GK_GPIO_UNDEFINED        }, \
    { GPIO_34, GK_GPIO_UNDEFINED        }, \
    { GPIO_35, GK_GPIO_UNDEFINED        }, \
    { GPIO_37, GK_GPIO_SPI0_SO          }, \
    { GPIO_38, GK_GPIO_SPI0_SI          }, \
    { GPIO_39, GK_GPIO_IR_CUT1        }, \
    { GPIO_40, GK_GPIO_IR_CUT2         }, \
    { GPIO_41, GK_GPIO_UNDEFINED        }, \
    { GPIO_42, GK_GPIO_UNDEFINED        }, \
    { GPIO_43, GK_GPIO_UNDEFINED        }, \
    { GPIO_44, GK_GPIO_SENSOR1_RESET    }, \
    { GPIO_45, GK_GPIO_UNDEFINED        }, \
    { GPIO_46, GK_GPIO_UNDEFINED        }, \
    { GPIO_47, GK_GPIO_UNDEFINED        }, \
    { GPIO_48, GK_GPIO_UNDEFINED        }, \
    { GPIO_49, GK_GPIO_UNDEFINED        }, \
    { GPIO_50, GK_GPIO_UNDEFINED        }, \
    { GPIO_51, GK_GPIO_UNDEFINED        }, \
    { GPIO_52, GK_GPIO_UNDEFINED        }, \
    { GPIO_53, GK_GPIO_UNDEFINED        }, \
    { GPIO_54, GK_GPIO_UNDEFINED        }, \
    { GPIO_55, GK_GPIO_UNDEFINED        }, \
    { GPIO_56, GK_GPIO_UNDEFINED        }, \
    { GPIO_57, GK_GPIO_UNDEFINED        }, \
    { GPIO_58, GK_GPIO_UNDEFINED        }, \
    { GPIO_59, GK_GPIO_UNDEFINED        }, \
    { GPIO_60, GK_GPIO_UNDEFINED        }, \
    { GPIO_61, GK_GPIO_I2C0_DATA        }, \
    { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
    { GPIO_63, GK_GPIO_I2C0_CLK         }, \
    { GPIO_64, GK_GPIO_UNDEFINED        }, \
    { GPIO_65, GK_GPIO_UNDEFINED        }, \
    { GPIO_66, GK_GPIO_UNDEFINED        }, \
    { GPIO_67, GK_GPIO_UNDEFINED        }, \
    { GPIO_68, GK_GPIO_UNDEFINED        }, \
    { GPIO_69, GK_GPIO_UNDEFINED        }, \
    { GPIO_70, GK_GPIO_UNDEFINED        }, \
    { GPIO_71, GK_GPIO_UNDEFINED        }, \
    { GPIO_72, GK_GPIO_UNDEFINED        }, \
    { GPIO_73, GK_GPIO_UNDEFINED        }, \
    { GPIO_74, GK_GPIO_UNDEFINED        }, \
    { GPIO_75, GK_GPIO_UNDEFINED        }, \
    { GPIO_76, GK_GPIO_UNDEFINED        }, \
    { GPIO_77, GK_GPIO_UNDEFINED        }, \
    { GPIO_78, GK_GPIO_UNDEFINED        }, \
    { GPIO_79, GK_GPIO_UNDEFINED        },
#endif /* CONFIG_MACH_GK_T26_R25_D */
//

#if defined(CONFIG_MACH_GK_T26_R25_E)
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_4 , GK_GPIO_SDIO1_WP_N       }, \
            { GPIO_5 , GK_GPIO_SDIO1_DATA_1     }, \
            { GPIO_6 , GK_GPIO_SDIO1_DATA_0     }, \
            { GPIO_7 , GK_GPIO_SDIO1_CLK        }, \
            { GPIO_8 , GK_GPIO_SDIO1_CMD        }, \
            { GPIO_9 , GK_GPIO_SDIO1_DATA_3     }, \
            { GPIO_10, GK_GPIO_SDIO1_DATA_2     }, \
            { GPIO_11, GK_GPIO_SDIO1_CD_N       }, \
            { GPIO_14, GK_GPIO_SDIO0_WP_N       }, \
            { GPIO_15, GK_GPIO_SDIO0_DATA_1     }, \
            { GPIO_16, GK_GPIO_SDIO0_DATA_0     }, \
            { GPIO_17, GK_GPIO_SDIO0_CMD        }, \
            { GPIO_18, GK_GPIO_SDIO0_DATA_3     }, \
            { GPIO_19, GK_GPIO_SDIO0_DATA_2     }, \
            { GPIO_20, GK_GPIO_SDIO0_CD_N       }, \
            { GPIO_22, GK_GPIO_SDIO0_CLK        },

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
            { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
            { GPIO_6 , GK_GPIO_UNDEFINED        }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
            { GPIO_9 , GK_GPIO_UNDEFINED        }, \
            { GPIO_10, GK_GPIO_UNDEFINED        }, \
            { GPIO_11, GK_GPIO_UNDEFINED        }, \
            { GPIO_14, GK_GPIO_UNDEFINED        }, \
            { GPIO_15, GK_GPIO_UNDEFINED        }, \
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_UNDEFINED        }, \
            { GPIO_20, GK_GPIO_UNDEFINED        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
            { GPIO_2 , GK_GPIO_UNDEFINED        }, \
            { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
            { GPIO_36,  GK_GPIO_PHY_DATA_LED     }, \
            { GPIO_23,  GK_GPIO_PHY_LINK_LED     },

#define SYSTEM_GPIO_XREF_TABLE             \
            { GPIO_0 , GK_GPIO_SF_CS0           }, \
            { GPIO_1 , GK_GPIO_SF_CS1       }, \
            { GPIO_2 , GK_GPIO_UNDEFINED           }, \
            { GPIO_3 , GK_GPIO_UNDEFINED       }, \
            SYSTEM_GPIO_SDCARD_TABLE\
            { GPIO_12, GK_GPIO_UNDEFINED        }, \
            { GPIO_13, GK_GPIO_UNDEFINED        }, \
            { GPIO_21, GK_GPIO_UNDEFINED        }, \
            { GPIO_24, GK_GPIO_OUTPUT_1 /* spk_en*/       }, \
            { GPIO_25, GK_GPIO_UART0_RX         }, \
            { GPIO_26, GK_GPIO_UART0_TX         }, \
            { GPIO_27, GK_GPIO_PWM_0        }, \
            { GPIO_28, GK_GPIO_INPUT_1        }, \
            { GPIO_29, GK_GPIO_SF_HOLD        }, \
            { GPIO_30, GK_GPIO_SF_WP        }, \
            { GPIO_31, GK_GPIO_UNDEFINED        }, \
            { GPIO_32, GK_GPIO_UNDEFINED        }, \
            { GPIO_33, GK_GPIO_UNDEFINED        }, \
            { GPIO_34, GK_GPIO_UNDEFINED        }, \
            { GPIO_35, GK_GPIO_UNDEFINED        }, \
            { GPIO_37, GK_GPIO_IR_CUT2          }, \
            { GPIO_38, GK_GPIO_UNDEFINED          }, \
            { GPIO_39, GK_GPIO_IR_CUT1        }, \
            { GPIO_40, GK_GPIO_UNDEFINED         }, \
            { GPIO_41, GK_GPIO_UNDEFINED        }, \
            { GPIO_42, GK_GPIO_UNDEFINED        }, \
            { GPIO_43, GK_GPIO_UNDEFINED        }, \
            { GPIO_44, GK_GPIO_SENSOR1_RESET    }, \
            { GPIO_45, GK_GPIO_UNDEFINED        }, \
            { GPIO_46, GK_GPIO_UNDEFINED        }, \
            { GPIO_47, GK_GPIO_UNDEFINED        }, \
            { GPIO_48, GK_GPIO_UNDEFINED        }, \
            { GPIO_49, GK_GPIO_UNDEFINED        }, \
            { GPIO_50, GK_GPIO_UNDEFINED        }, \
            { GPIO_51, GK_GPIO_UNDEFINED        }, \
            { GPIO_52, GK_GPIO_UNDEFINED        }, \
            { GPIO_53, GK_GPIO_UNDEFINED        }, \
            { GPIO_54, GK_GPIO_UNDEFINED        }, \
            { GPIO_55, GK_GPIO_UNDEFINED        }, \
            { GPIO_56, GK_GPIO_UNDEFINED        }, \
            { GPIO_57, GK_GPIO_UNDEFINED        }, \
            { GPIO_58, GK_GPIO_UNDEFINED        }, \
            { GPIO_59, GK_GPIO_UNDEFINED        }, \
            { GPIO_60, GK_GPIO_UNDEFINED        }, \
            { GPIO_61, GK_GPIO_I2C0_DATA        }, \
            { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
            { GPIO_63, GK_GPIO_I2C0_CLK         }, \
            { GPIO_64, GK_GPIO_UNDEFINED        }, \
            { GPIO_65, GK_GPIO_UNDEFINED        }, \
            { GPIO_66, GK_GPIO_UNDEFINED        }, \
            { GPIO_67, GK_GPIO_UNDEFINED        }, \
            { GPIO_68, GK_GPIO_UNDEFINED        }, \
            { GPIO_69, GK_GPIO_UNDEFINED        }, \
            { GPIO_70, GK_GPIO_UNDEFINED        }, \
            { GPIO_71, GK_GPIO_UNDEFINED        }, \
            { GPIO_72, GK_GPIO_UNDEFINED        }, \
            { GPIO_73, GK_GPIO_UNDEFINED        }, \
            { GPIO_74, GK_GPIO_UNDEFINED        }, \
            { GPIO_75, GK_GPIO_UNDEFINED        }, \
            { GPIO_76, GK_GPIO_UNDEFINED        }, \
            { GPIO_77, GK_GPIO_UNDEFINED        }, \
            { GPIO_78, GK_GPIO_UNDEFINED        }, \
            { GPIO_79, GK_GPIO_UNDEFINED        },
#endif /* CONFIG_MACH_GK_T26_R25_E */

#if defined(CONFIG_MACH_GK_T26_R25_F)
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_9 , GK_GPIO_SDIO0_DATA_2     }, \
            { GPIO_10, GK_GPIO_SDIO0_DATA_3     }, \
            { GPIO_11, GK_GPIO_SDIO0_CMD       }, \
            { GPIO_12, GK_GPIO_SDIO0_CLK       }, \
            { GPIO_13, GK_GPIO_SDIO0_DATA_0     }, \
            { GPIO_14, GK_GPIO_SDIO0_DATA_1     }, \
            { GPIO_15, GK_GPIO_SDIO0_CD_N        }, 

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
            { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
            { GPIO_6 , GK_GPIO_UNDEFINED        }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
            { GPIO_9 , GK_GPIO_UNDEFINED        }, \
            { GPIO_10, GK_GPIO_UNDEFINED        }, \
            { GPIO_11, GK_GPIO_UNDEFINED        }, \
            { GPIO_14, GK_GPIO_UNDEFINED        }, \
            { GPIO_15, GK_GPIO_UNDEFINED        }, \
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_UNDEFINED        }, \
            { GPIO_20, GK_GPIO_UNDEFINED        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
            { GPIO_2 , GK_GPIO_UNDEFINED        }, \
            { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
            { GPIO_24,  GK_GPIO_PHY_DATA_LED     }, \
            { GPIO_3,  GK_GPIO_PHY_LINK_LED     },

#define SYSTEM_GPIO_XREF_TABLE             \
            { GPIO_0 , GK_GPIO_SF_CS0           }, \
            { GPIO_1 , GK_GPIO_SF_CS1       }, \
            { GPIO_2 , GK_GPIO_OUTPUT_1           }, \
            { GPIO_3 , GK_GPIO_UNDEFINED       }, \
            { GPIO_4 , GK_GPIO_UNDEFINED       }, \
            { GPIO_5 , GK_GPIO_UNDEFINED     }, \
            { GPIO_6 , GK_GPIO_UNDEFINED     }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
            SYSTEM_GPIO_SDCARD_TABLE\
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_OUTPUT_0 /* wifi_en*/}, \
            { GPIO_20, GK_GPIO_I2C1_DATA        }, \
            { GPIO_21, GK_GPIO_I2C1_CLK        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },  \
            { GPIO_23, GK_GPIO_UNDEFINED        }, \
            { GPIO_25, GK_GPIO_UART0_RX         }, \
            { GPIO_26, GK_GPIO_UART0_TX         }, \
            { GPIO_27, GK_GPIO_IR_CUT1       }, \
            { GPIO_28, GK_GPIO_IR_CUT2        }, \
            { GPIO_29, GK_GPIO_SF_HOLD}, \
            { GPIO_30, GK_GPIO_SF_WP}, \
            { GPIO_31, GK_GPIO_UNDEFINED        }, \
            { GPIO_32, GK_GPIO_OUTPUT_1        }, \
            { GPIO_33, GK_GPIO_OUTPUT_0        }, \
            { GPIO_34, GK_GPIO_OUTPUT_0        }, \
            { GPIO_35, GK_GPIO_OUTPUT_0        }, \
            { GPIO_36, GK_GPIO_OUTPUT_0        }, \
            { GPIO_37, GK_GPIO_OUTPUT_0          }, \
            { GPIO_38, GK_GPIO_OUTPUT_0          }, \
			{ GPIO_39, GK_GPIO_OUTPUT_0        }, \
            { GPIO_40, GK_GPIO_OUTPUT_0        }, \
            { GPIO_41, GK_GPIO_UNDEFINED        }, \
            { GPIO_42, GK_GPIO_UNDEFINED        }, \
            { GPIO_43, GK_GPIO_UNDEFINED        }, \
            { GPIO_44, GK_GPIO_UNDEFINED    }, \
            { GPIO_45, GK_GPIO_UNDEFINED        }, \
            { GPIO_46, GK_GPIO_UNDEFINED        }, \
            { GPIO_47, GK_GPIO_UNDEFINED        }, \
            { GPIO_48, GK_GPIO_UNDEFINED        }, \
            { GPIO_49, GK_GPIO_UNDEFINED        }, \
            { GPIO_50, GK_GPIO_UNDEFINED        }, \
            { GPIO_51, GK_GPIO_UNDEFINED        }, \
            { GPIO_52, GK_GPIO_UNDEFINED        }, \
            { GPIO_53, GK_GPIO_UNDEFINED        }, \
            { GPIO_54, GK_GPIO_UNDEFINED        }, \
            { GPIO_55, GK_GPIO_UNDEFINED        }, \
            { GPIO_56, GK_GPIO_UNDEFINED        }, \
            { GPIO_57, GK_GPIO_UNDEFINED        }, \
            { GPIO_58, GK_GPIO_UNDEFINED        }, \
            { GPIO_59, GK_GPIO_UNDEFINED        }, \
            { GPIO_60, GK_GPIO_UNDEFINED        }, \
            { GPIO_61, GK_GPIO_I2C0_DATA        }, \
            { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
            { GPIO_63, GK_GPIO_I2C0_CLK         }, \
            { GPIO_64, GK_GPIO_UNDEFINED        }, \
            { GPIO_65, GK_GPIO_UNDEFINED        }, \
            { GPIO_66, GK_GPIO_UNDEFINED        }, \
            { GPIO_67, GK_GPIO_UNDEFINED        }, \
            { GPIO_68, GK_GPIO_UNDEFINED        }, \
            { GPIO_69, GK_GPIO_UNDEFINED        }, \
            { GPIO_70, GK_GPIO_UNDEFINED        }, \
            { GPIO_71, GK_GPIO_UNDEFINED        }, \
            { GPIO_72, GK_GPIO_UNDEFINED        }, \
            { GPIO_73, GK_GPIO_UNDEFINED        }, \
            { GPIO_74, GK_GPIO_UNDEFINED        }, \
            { GPIO_75, GK_GPIO_UNDEFINED        }, \
            { GPIO_76, GK_GPIO_UNDEFINED        }, \
            { GPIO_77, GK_GPIO_UNDEFINED        }, \
            { GPIO_78, GK_GPIO_UNDEFINED        }, \
            { GPIO_79, GK_GPIO_UNDEFINED        },
#endif /* CONFIG_MACH_GK_T26_R25_F */




#if defined(CONFIG_MACH_GK_T26_R25_G)
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_9 , GK_GPIO_SDIO0_CD_N     }, \
            { GPIO_10, GK_GPIO_SDIO0_DATA_1     }, \
            { GPIO_11, GK_GPIO_SDIO0_DATA_0       }, \
            { GPIO_12, GK_GPIO_SDIO0_CLK       }, \
            { GPIO_13, GK_GPIO_SDIO0_CMD     }, \
            { GPIO_14, GK_GPIO_SDIO0_DATA_3     }, \
            { GPIO_15, GK_GPIO_SDIO0_DATA_2        }, 
  

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_4 , GK_GPIO_UNDEFINED        }, \
            { GPIO_5 , GK_GPIO_UNDEFINED         }, \
            { GPIO_6 , GK_GPIO_UNDEFINED        }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
            { GPIO_9 , GK_GPIO_UNDEFINED        }, \
            { GPIO_10, GK_GPIO_UNDEFINED        }, \
            { GPIO_11, GK_GPIO_UNDEFINED        }, \
            { GPIO_14, GK_GPIO_UNDEFINED        }, \
            { GPIO_15, GK_GPIO_UNDEFINED        }, \
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_UNDEFINED        }, \
            { GPIO_20, GK_GPIO_UNDEFINED        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
            { GPIO_2 , GK_GPIO_UNDEFINED        }, \
            { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
            { GPIO_40,  GK_GPIO_PHY_DATA_LED     }, \
            { GPIO_15,  GK_GPIO_PHY_LINK_LED     },

#define SYSTEM_GPIO_XREF_TABLE             \
            { GPIO_0 , GK_GPIO_SF_CS0           }, \
            { GPIO_1 , GK_GPIO_SF_CS1       }, \
            { GPIO_2 , GK_GPIO_UNDEFINED           }, \
            { GPIO_3 , GK_GPIO_UNDEFINED       }, \
            { GPIO_4 , GK_GPIO_UNDEFINED       }, \
            { GPIO_5 , GK_GPIO_UNDEFINED     }, \
            { GPIO_6 , GK_GPIO_UNDEFINED     }, \
            { GPIO_7 , GK_GPIO_UART1_RX        }, \
            { GPIO_8 , GK_GPIO_UART1_TX        }, \            
            SYSTEM_GPIO_SDCARD_TABLE\
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_OUTPUT_0        }, \
            { GPIO_20, GK_GPIO_OUTPUT_1        }, \
            { GPIO_21, GK_GPIO_OUTPUT_1        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },  \
            { GPIO_23, GK_GPIO_UNDEFINED        }, \            
            { GPIO_24, GK_GPIO_UNDEFINED        }, \
            { GPIO_25, GK_GPIO_UART0_RX         }, \
            { GPIO_26, GK_GPIO_UART0_TX         }, \
            { GPIO_27, GK_GPIO_UNDEFINED       }, \
            { GPIO_28, GK_GPIO_UNDEFINED        }, \
            { GPIO_29, GK_GPIO_SF_HOLD}, \
            { GPIO_30, GK_GPIO_SF_WP}, \
            { GPIO_31, GK_GPIO_UNDEFINED        }, \
            { GPIO_32, GK_GPIO_UNDEFINED        }, \
            { GPIO_33, GK_GPIO_UNDEFINED        }, \
            { GPIO_34, GK_GPIO_UNDEFINED        }, \
            { GPIO_35, GK_GPIO_UNDEFINED        }, \
            { GPIO_37, GK_GPIO_UNDEFINED          }, \
            { GPIO_38, GK_GPIO_OUTPUT_0          }, \
            { GPIO_41, GK_GPIO_UNDEFINED        }, \
            { GPIO_42, GK_GPIO_UNDEFINED        }, \
            { GPIO_43, GK_GPIO_UNDEFINED        }, \
            { GPIO_44, GK_GPIO_IR_CUT1    }, \
            { GPIO_45, GK_GPIO_IR_CUT2        }, \
            { GPIO_46, GK_GPIO_UNDEFINED        }, \
            { GPIO_47, GK_GPIO_UNDEFINED        }, \
            { GPIO_48, GK_GPIO_UNDEFINED        }, \
            { GPIO_49, GK_GPIO_UART2_TX        }, \
            { GPIO_50, GK_GPIO_UART2_RX        }, \
            { GPIO_51, GK_GPIO_UNDEFINED        }, \
            { GPIO_52, GK_GPIO_UNDEFINED        }, \
            { GPIO_53, GK_GPIO_UNDEFINED        }, \
            { GPIO_54, GK_GPIO_UNDEFINED        }, \
            { GPIO_55, GK_GPIO_UNDEFINED        }, \
            { GPIO_56, GK_GPIO_UNDEFINED        }, \
            { GPIO_57, GK_GPIO_UNDEFINED        }, \
            { GPIO_58, GK_GPIO_UNDEFINED        }, \
            { GPIO_59, GK_GPIO_UNDEFINED        }, \
            { GPIO_60, GK_GPIO_UNDEFINED        }, \
            { GPIO_61, GK_GPIO_I2C0_DATA        }, \
            { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
            { GPIO_63, GK_GPIO_I2C0_CLK         }, \
            { GPIO_64, GK_GPIO_UNDEFINED        }, \
            { GPIO_65, GK_GPIO_UNDEFINED        }, \
            { GPIO_66, GK_GPIO_UNDEFINED        }, \
            { GPIO_67, GK_GPIO_UNDEFINED        }, \
            { GPIO_68, GK_GPIO_UNDEFINED        }, \
            { GPIO_69, GK_GPIO_UNDEFINED        }, \
            { GPIO_70, GK_GPIO_UNDEFINED        }, \
            { GPIO_71, GK_GPIO_UNDEFINED        }, \
            { GPIO_72, GK_GPIO_UNDEFINED        }, \
            { GPIO_73, GK_GPIO_UNDEFINED        }, \
            { GPIO_74, GK_GPIO_UNDEFINED        }, \
            { GPIO_75, GK_GPIO_UNDEFINED        }, \
            { GPIO_76, GK_GPIO_UNDEFINED        }, \
            { GPIO_77, GK_GPIO_UNDEFINED        }, \
            { GPIO_78, GK_GPIO_UNDEFINED        }, \
            { GPIO_79, GK_GPIO_UNDEFINED        },
#endif /* CONFIG_MACH_GK_T26_R25_G */

#if defined(CONFIG_MACH_GK_T26_R25_H)
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_9 , GK_GPIO_UNDEFINED     }, \
            { GPIO_10, GK_GPIO_SDIO0_DATA_1     }, \
            { GPIO_11, GK_GPIO_SDIO0_DATA_0       }, \
            { GPIO_12, GK_GPIO_SDIO0_CLK       }, \
            { GPIO_13, GK_GPIO_SDIO0_CMD     }, \
            { GPIO_14, GK_GPIO_SDIO0_DATA_2     }, \
            { GPIO_15, GK_GPIO_SDIO0_CD_N        }, 

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
            { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
            { GPIO_6 , GK_GPIO_UNDEFINED        }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
            { GPIO_9 , GK_GPIO_UNDEFINED        }, \
            { GPIO_10, GK_GPIO_UNDEFINED        }, \
            { GPIO_11, GK_GPIO_UNDEFINED        }, \
            { GPIO_14, GK_GPIO_UNDEFINED        }, \
            { GPIO_15, GK_GPIO_UNDEFINED        }, \
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_UNDEFINED        }, \
            { GPIO_20, GK_GPIO_UNDEFINED        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
            { GPIO_2 , GK_GPIO_UNDEFINED        }, \
            { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
            { GPIO_37,  GK_GPIO_PHY_DATA_LED     }, \
            { GPIO_38,  GK_GPIO_PHY_LINK_LED     },

#define SYSTEM_GPIO_XREF_TABLE             \
            { GPIO_0 , GK_GPIO_SF_CS0           }, \
            { GPIO_1 , GK_GPIO_SF_CS1       }, \
            { GPIO_2 , GK_GPIO_UNDEFINED           }, \
            { GPIO_3 , GK_GPIO_I2C1_DATA       }, \
            { GPIO_4 , GK_GPIO_I2C1_CLK       }, \
            { GPIO_5 , GK_GPIO_UNDEFINED     }, \
            { GPIO_6 , GK_GPIO_UNDEFINED     }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
            SYSTEM_GPIO_SDCARD_TABLE\
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_SDIO0_DATA_3     }, \
            { GPIO_20, GK_GPIO_UNDEFINED        }, \
            { GPIO_21, GK_GPIO_UNDEFINED        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },  \
            { GPIO_23, GK_GPIO_UNDEFINED        }, \
            { GPIO_25, GK_GPIO_UART0_RX         }, \
            { GPIO_26, GK_GPIO_UART0_TX         }, \
            { GPIO_27, GK_GPIO_UNDEFINED       }, \
            { GPIO_28, GK_GPIO_UNDEFINED        }, \
            { GPIO_29, GK_GPIO_SF_HOLD}, \
            { GPIO_30, GK_GPIO_SF_WP}, \
            { GPIO_31, GK_GPIO_UNDEFINED        }, \
            { GPIO_32, GK_GPIO_INPUT_1        }, \
            { GPIO_33, GK_GPIO_OUTPUT_1        }, \
            { GPIO_34, GK_GPIO_OUTPUT_1        }, \
            { GPIO_35, GK_GPIO_OUTPUT_1        }, \
			{ GPIO_36, GK_GPIO_OUTPUT_0 /* wifi_en*/}, \
            { GPIO_37, GK_GPIO_UNDEFINED          }, \
            { GPIO_38, GK_GPIO_UNDEFINED          }, \
			{ GPIO_39, GK_GPIO_IR_CUT1        }, \
            { GPIO_40, GK_GPIO_IR_CUT2        }, \
            { GPIO_41, GK_GPIO_UNDEFINED        }, \
            { GPIO_42, GK_GPIO_UNDEFINED        }, \
            { GPIO_43, GK_GPIO_UNDEFINED        }, \
            { GPIO_44, GK_GPIO_UNDEFINED    }, \
            { GPIO_45, GK_GPIO_UNDEFINED        }, \
            { GPIO_46, GK_GPIO_UNDEFINED        }, \
            { GPIO_47, GK_GPIO_UNDEFINED        }, \
            { GPIO_48, GK_GPIO_UNDEFINED        }, \
            { GPIO_49, GK_GPIO_UNDEFINED        }, \
            { GPIO_50, GK_GPIO_UNDEFINED        }, \
            { GPIO_51, GK_GPIO_UNDEFINED        }, \
            { GPIO_52, GK_GPIO_UNDEFINED        }, \
            { GPIO_53, GK_GPIO_UNDEFINED        }, \
            { GPIO_54, GK_GPIO_UNDEFINED        }, \
            { GPIO_55, GK_GPIO_UNDEFINED        }, \
            { GPIO_56, GK_GPIO_UNDEFINED        }, \
            { GPIO_57, GK_GPIO_UNDEFINED        }, \
            { GPIO_58, GK_GPIO_UNDEFINED        }, \
            { GPIO_59, GK_GPIO_UNDEFINED        }, \
            { GPIO_60, GK_GPIO_UNDEFINED        }, \
            { GPIO_61, GK_GPIO_I2C0_DATA        }, \
            { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
            { GPIO_63, GK_GPIO_I2C0_CLK         }, \
            { GPIO_64, GK_GPIO_UNDEFINED        }, \
            { GPIO_65, GK_GPIO_UNDEFINED        }, \
            { GPIO_66, GK_GPIO_UNDEFINED        }, \
            { GPIO_67, GK_GPIO_UNDEFINED        }, \
            { GPIO_68, GK_GPIO_UNDEFINED        }, \
            { GPIO_69, GK_GPIO_UNDEFINED        }, \
            { GPIO_70, GK_GPIO_UNDEFINED        }, \
            { GPIO_71, GK_GPIO_UNDEFINED        }, \
            { GPIO_72, GK_GPIO_UNDEFINED        }, \
            { GPIO_73, GK_GPIO_UNDEFINED        }, \
            { GPIO_74, GK_GPIO_UNDEFINED        }, \
            { GPIO_75, GK_GPIO_UNDEFINED        }, \
            { GPIO_76, GK_GPIO_UNDEFINED        }, \
            { GPIO_77, GK_GPIO_UNDEFINED        }, \
            { GPIO_78, GK_GPIO_UNDEFINED        }, \
            { GPIO_79, GK_GPIO_UNDEFINED        },
#endif /* CONFIG_MACH_GK_T26_R25_H */

#if defined(CONFIG_MACH_GK_T26_R25_I)
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_28 , GK_GPIO_SDIO0_DATA_2     }, \
            { GPIO_27, GK_GPIO_SDIO0_DATA_3     }, \
            { GPIO_20, GK_GPIO_SDIO0_CMD       }, \
            { GPIO_15, GK_GPIO_SDIO0_CLK       }, \
            { GPIO_14, GK_GPIO_SDIO0_DATA_0     }, \
            { GPIO_12, GK_GPIO_SDIO0_DATA_1     }, \
            { GPIO_10, GK_GPIO_SDIO0_CD_N        }, 

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
            { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
            { GPIO_6 , GK_GPIO_UNDEFINED        }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
            { GPIO_9 , GK_GPIO_UNDEFINED        }, \
            { GPIO_10, GK_GPIO_UNDEFINED        }, \
            { GPIO_11, GK_GPIO_UNDEFINED        }, \
            { GPIO_14, GK_GPIO_UNDEFINED        }, \
            { GPIO_15, GK_GPIO_UNDEFINED        }, \
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_UNDEFINED        }, \
            { GPIO_20, GK_GPIO_UNDEFINED        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
            { GPIO_2 , GK_GPIO_UNDEFINED        }, \
            { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
            { GPIO_24,  GK_GPIO_PHY_DATA_LED     }, \
            { GPIO_38,  GK_GPIO_PHY_LINK_LED     },

#define SYSTEM_GPIO_XREF_TABLE             \
            { GPIO_0 , GK_GPIO_SF_CS0           }, \
            { GPIO_1 , GK_GPIO_SF_CS1       }, \
            { GPIO_2 , GK_GPIO_OUTPUT_1           }, \
            { GPIO_3 , GK_GPIO_I2C1_CLK       }, \
            { GPIO_4 , GK_GPIO_I2C1_DATA       }, \
            { GPIO_5 , GK_GPIO_UNDEFINED     }, \
            { GPIO_6 , GK_GPIO_UNDEFINED     }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
			{ GPIO_9 , GK_GPIO_OUTPUT_0        }, \
            SYSTEM_GPIO_SDCARD_TABLE\
			{ GPIO_11 , GK_GPIO_OUTPUT_0        }, \
			{ GPIO_13 , GK_GPIO_OUTPUT_0        }, \
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_OUTPUT_0			}, \
            { GPIO_21, GK_GPIO_OUTPUT_0        }, \
            { GPIO_22, GK_GPIO_OUTPUT_0        },  \
            { GPIO_23, GK_GPIO_OUTPUT_0        }, \
			{ GPIO_24, GK_GPIO_OUTPUT_0        }, \
            { GPIO_25, GK_GPIO_UART0_RX         }, \
            { GPIO_26, GK_GPIO_UART0_TX         }, \
            { GPIO_29, GK_GPIO_SF_HOLD}, \
            { GPIO_30, GK_GPIO_SF_WP}, \
            { GPIO_31, GK_GPIO_UNDEFINED        }, \
            { GPIO_32, GK_GPIO_OUTPUT_0        }, \
            { GPIO_33, GK_GPIO_OUTPUT_0        }, \
            { GPIO_34, GK_GPIO_UNDEFINED        }, \
            { GPIO_35, GK_GPIO_OUTPUT_1        }, \
            { GPIO_36, GK_GPIO_OUTPUT_0        }, \
            { GPIO_37, GK_GPIO_OUTPUT_0          }, \
			{ GPIO_39, GK_GPIO_IR_CUT2        }, \
            { GPIO_40, GK_GPIO_IR_CUT1        }, \
            { GPIO_41, GK_GPIO_UNDEFINED        }, \
            { GPIO_42, GK_GPIO_UNDEFINED        }, \
            { GPIO_43, GK_GPIO_UNDEFINED        }, \
            { GPIO_44, GK_GPIO_UNDEFINED    }, \
            { GPIO_45, GK_GPIO_UNDEFINED        }, \
            { GPIO_46, GK_GPIO_UNDEFINED        }, \
            { GPIO_47, GK_GPIO_UNDEFINED        }, \
            { GPIO_48, GK_GPIO_UNDEFINED        }, \
            { GPIO_49, GK_GPIO_UNDEFINED        }, \
            { GPIO_50, GK_GPIO_UNDEFINED        }, \
            { GPIO_51, GK_GPIO_UNDEFINED        }, \
            { GPIO_52, GK_GPIO_UNDEFINED        }, \
            { GPIO_53, GK_GPIO_UNDEFINED        }, \
            { GPIO_54, GK_GPIO_UNDEFINED        }, \
            { GPIO_55, GK_GPIO_UNDEFINED        }, \
            { GPIO_56, GK_GPIO_UNDEFINED        }, \
            { GPIO_57, GK_GPIO_UNDEFINED        }, \
            { GPIO_58, GK_GPIO_UNDEFINED        }, \
            { GPIO_59, GK_GPIO_UNDEFINED        }, \
            { GPIO_60, GK_GPIO_UNDEFINED        }, \
            { GPIO_61, GK_GPIO_I2C0_DATA        }, \
            { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
            { GPIO_63, GK_GPIO_I2C0_CLK         }, \
            { GPIO_64, GK_GPIO_UNDEFINED        }, \
            { GPIO_65, GK_GPIO_UNDEFINED        }, \
            { GPIO_66, GK_GPIO_UNDEFINED        }, \
            { GPIO_67, GK_GPIO_UNDEFINED        }, \
            { GPIO_68, GK_GPIO_UNDEFINED        }, \
            { GPIO_69, GK_GPIO_UNDEFINED        }, \
            { GPIO_70, GK_GPIO_UNDEFINED        }, \
            { GPIO_71, GK_GPIO_UNDEFINED        }, \
            { GPIO_72, GK_GPIO_UNDEFINED        }, \
            { GPIO_73, GK_GPIO_UNDEFINED        }, \
            { GPIO_74, GK_GPIO_UNDEFINED        }, \
            { GPIO_75, GK_GPIO_UNDEFINED        }, \
            { GPIO_76, GK_GPIO_UNDEFINED        }, \
            { GPIO_77, GK_GPIO_UNDEFINED        }, \
            { GPIO_78, GK_GPIO_UNDEFINED        }, \
            { GPIO_79, GK_GPIO_UNDEFINED        },
#endif /* CONFIG_MACH_GK_T26_R25_I */

#if defined(CONFIG_MACH_GK_T26_R25_J)
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_9 , GK_GPIO_UNDEFINED     }, \
            { GPIO_10, GK_GPIO_SDIO0_DATA_2      }, \
            { GPIO_11, GK_GPIO_SDIO0_DATA_3       }, \
            { GPIO_12, GK_GPIO_SDIO0_CMD       }, \
            { GPIO_13, GK_GPIO_SDIO0_DATA_0     }, \
            { GPIO_14, GK_GPIO_SDIO0_CLK     }, \
            { GPIO_15, GK_GPIO_SDIO0_CD_N        },

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
            { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
            { GPIO_6 , GK_GPIO_UNDEFINED        }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
            { GPIO_9 , GK_GPIO_UNDEFINED        }, \
            { GPIO_10, GK_GPIO_UNDEFINED        }, \
            { GPIO_11, GK_GPIO_UNDEFINED        }, \
            { GPIO_14, GK_GPIO_UNDEFINED        }, \
            { GPIO_15, GK_GPIO_UNDEFINED        }, \
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_UNDEFINED        }, \
            { GPIO_20, GK_GPIO_UNDEFINED        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
            { GPIO_2 , GK_GPIO_UNDEFINED        }, \
            { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
            { GPIO_37,  GK_GPIO_UNDEFINED     }, \
            { GPIO_38,  GK_GPIO_PHY_LINK_LED     },

#define SYSTEM_GPIO_XREF_TABLE             \
            { GPIO_0 , GK_GPIO_SF_CS0           }, \
            { GPIO_1 , GK_GPIO_SF_CS1       }, \
            { GPIO_2 , GK_GPIO_UNDEFINED           }, \
            { GPIO_3 , GK_GPIO_I2C1_DATA       }, \
            { GPIO_4 , GK_GPIO_I2C1_CLK       }, \
            { GPIO_5 , GK_GPIO_UNDEFINED     }, \
            { GPIO_6 , GK_GPIO_UNDEFINED     }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
            SYSTEM_GPIO_SDCARD_TABLE\
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_SDIO0_DATA_1     }, \
            { GPIO_20, GK_GPIO_UNDEFINED        }, \
            { GPIO_21, GK_GPIO_UNDEFINED        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },  \
            { GPIO_23, GK_GPIO_UNDEFINED        }, \
            { GPIO_25, GK_GPIO_UART0_RX         }, \
            { GPIO_26, GK_GPIO_UART0_TX         }, \
            { GPIO_27, GK_GPIO_UNDEFINED       }, \
            { GPIO_28, GK_GPIO_UNDEFINED        }, \
            { GPIO_29, GK_GPIO_SF_HOLD}, \
            { GPIO_30, GK_GPIO_SF_WP}, \
            { GPIO_31, GK_GPIO_UNDEFINED        }, \
            { GPIO_32, GK_GPIO_INPUT_1        }, \
            { GPIO_33, GK_GPIO_PWM_0        /*GK_GPIO_OUTPUT_0 GK_GPIO_PWM_0*/}, \ 
            { GPIO_34, GK_GPIO_PWM_1       }, \
            { GPIO_35, GK_GPIO_OUTPUT_1        }, \
			{ GPIO_36, GK_GPIO_OUTPUT_0 /* wifi_en*/}, \
            { GPIO_37, GK_GPIO_UNDEFINED          }, \
            { GPIO_38, GK_GPIO_UNDEFINED          }, \
			{ GPIO_39, GK_GPIO_IR_CUT2        }, \
            { GPIO_40, GK_GPIO_IR_CUT1        }, \
            { GPIO_41, GK_GPIO_UNDEFINED        }, \
            { GPIO_42, GK_GPIO_UNDEFINED        }, \
            { GPIO_43, GK_GPIO_UNDEFINED        }, \
            { GPIO_44, GK_GPIO_UNDEFINED    }, \
            { GPIO_45, GK_GPIO_UNDEFINED        }, \
            { GPIO_46, GK_GPIO_UNDEFINED        }, \
            { GPIO_47, GK_GPIO_UNDEFINED        }, \
            { GPIO_48, GK_GPIO_UNDEFINED        }, \
            { GPIO_49, GK_GPIO_UNDEFINED        }, \
            { GPIO_50, GK_GPIO_UNDEFINED        }, \
            { GPIO_51, GK_GPIO_UNDEFINED        }, \
            { GPIO_52, GK_GPIO_UNDEFINED        }, \
            { GPIO_53, GK_GPIO_UNDEFINED        }, \
            { GPIO_54, GK_GPIO_UNDEFINED        }, \
            { GPIO_55, GK_GPIO_UNDEFINED        }, \
            { GPIO_56, GK_GPIO_UNDEFINED        }, \
            { GPIO_57, GK_GPIO_UNDEFINED        }, \
            { GPIO_58, GK_GPIO_UNDEFINED        }, \
            { GPIO_59, GK_GPIO_UNDEFINED        }, \
            { GPIO_60, GK_GPIO_UNDEFINED        }, \
            { GPIO_61, GK_GPIO_I2C0_DATA        }, \
            { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
            { GPIO_63, GK_GPIO_I2C0_CLK         }, \
            { GPIO_64, GK_GPIO_UNDEFINED        }, \
            { GPIO_65, GK_GPIO_UNDEFINED        }, \
            { GPIO_66, GK_GPIO_UNDEFINED        }, \
            { GPIO_67, GK_GPIO_UNDEFINED        }, \
            { GPIO_68, GK_GPIO_UNDEFINED        }, \
            { GPIO_69, GK_GPIO_UNDEFINED        }, \
            { GPIO_70, GK_GPIO_UNDEFINED        }, \
            { GPIO_71, GK_GPIO_UNDEFINED        }, \
            { GPIO_72, GK_GPIO_UNDEFINED        }, \
            { GPIO_73, GK_GPIO_UNDEFINED        }, \
            { GPIO_74, GK_GPIO_UNDEFINED        }, \
            { GPIO_75, GK_GPIO_UNDEFINED        }, \
            { GPIO_76, GK_GPIO_UNDEFINED        }, \
            { GPIO_77, GK_GPIO_UNDEFINED        }, \
            { GPIO_78, GK_GPIO_UNDEFINED        }, \
            { GPIO_79, GK_GPIO_UNDEFINED        },
#endif /* CONFIG_MACH_GK_T26_R25_J */

#if defined(CONFIG_MACH_GK_T26_R25_K)
#ifdef CONFIG_SYSTEM_USE_SDCARD
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_24, GK_GPIO_SDIO0_DATA_0     }, \
            { GPIO_35, GK_GPIO_SDIO0_DATA_1      }, \
            { GPIO_8,  GK_GPIO_SDIO0_DATA_2      }, \
            { GPIO_15, GK_GPIO_SDIO0_DATA_3      }, \
            { GPIO_21, GK_GPIO_SDIO0_CLK        }, \
            { GPIO_18, GK_GPIO_SDIO0_CMD        }, \
            { GPIO_31, GK_GPIO_SDIO0_CD_N        },

#else
#define SYSTEM_GPIO_SDCARD_TABLE        \
            { GPIO_4 , GK_GPIO_I2C1_DATA        }, \
            { GPIO_5 , GK_GPIO_I2C1_CLK         }, \
            { GPIO_6 , GK_GPIO_UNDEFINED        }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            { GPIO_8 , GK_GPIO_UNDEFINED        }, \
            { GPIO_9 , GK_GPIO_UNDEFINED        }, \
            { GPIO_10, GK_GPIO_UNDEFINED        }, \
            { GPIO_11, GK_GPIO_UNDEFINED        }, \
            { GPIO_14, GK_GPIO_UNDEFINED        }, \
            { GPIO_15, GK_GPIO_UNDEFINED        }, \
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_18, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_UNDEFINED        }, \
            { GPIO_20, GK_GPIO_UNDEFINED        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },
#endif

#define SYSTEM_GPIO_EXTPHY_TABLE           \
            { GPIO_2 , GK_GPIO_UNDEFINED        }, \
            { GPIO_3 , GK_GPIO_UNDEFINED        },

#define SYSTEM_GPIO_INTPHY_TABLE           \
            { GPIO_3,  GK_GPIO_PHY_LINK_LED     },

#define SYSTEM_GPIO_XREF_TABLE             \
            { GPIO_0 , GK_GPIO_SF_CS0           }, \
            { GPIO_1 , GK_GPIO_SF_CS1       }, \
            { GPIO_2 , GK_GPIO_UNDEFINED           }, \
            { GPIO_4 , GK_GPIO_UNDEFINED       }, \
            { GPIO_5 , GK_GPIO_OUTPUT_0     }, \
            { GPIO_6 , GK_GPIO_UNDEFINED     }, \
            { GPIO_7 , GK_GPIO_UNDEFINED        }, \
            SYSTEM_GPIO_SDCARD_TABLE\
            { GPIO_16, GK_GPIO_UNDEFINED        }, \
            { GPIO_17, GK_GPIO_UNDEFINED        }, \
            { GPIO_19, GK_GPIO_UNDEFINED     }, \
            { GPIO_20, GK_GPIO_UNDEFINED        }, \
            { GPIO_22, GK_GPIO_UNDEFINED        },  \
            { GPIO_23, GK_GPIO_UNDEFINED        }, \
            { GPIO_25, GK_GPIO_UART0_RX         }, \
            { GPIO_26, GK_GPIO_UART0_TX         }, \
            { GPIO_27, GK_GPIO_UNDEFINED       }, \
            { GPIO_28, GK_GPIO_UNDEFINED        }, \
            { GPIO_29, GK_GPIO_SF_HOLD}, \
            { GPIO_30, GK_GPIO_SF_WP}, \
            { GPIO_32, GK_GPIO_PWM_1        }, \
            { GPIO_33, GK_GPIO_UNDEFINED        /*GK_GPIO_OUTPUT_0 GK_GPIO_PWM_0*/}, \ 
            { GPIO_34, GK_GPIO_PWM_0       }, \
			{ GPIO_36, GK_GPIO_I2C1_CLK }, \
            { GPIO_37, GK_GPIO_I2C1_DATA          }, \
            { GPIO_38, GK_GPIO_UNDEFINED          }, \
			{ GPIO_39, GK_GPIO_IR_CUT1        }, \
            { GPIO_40, GK_GPIO_IR_CUT2        }, \
            { GPIO_41, GK_GPIO_OUTPUT_1        }, \
            { GPIO_42, GK_GPIO_UNDEFINED        }, \
            { GPIO_43, GK_GPIO_UNDEFINED        }, \
            { GPIO_44, GK_GPIO_UNDEFINED    }, \
            { GPIO_45, GK_GPIO_UNDEFINED        }, \
            { GPIO_46, GK_GPIO_UNDEFINED        }, \
            { GPIO_47, GK_GPIO_UNDEFINED        }, \
            { GPIO_48, GK_GPIO_UNDEFINED        }, \
            { GPIO_49, GK_GPIO_UNDEFINED        }, \
            { GPIO_50, GK_GPIO_UNDEFINED        }, \
            { GPIO_51, GK_GPIO_UNDEFINED        }, \
            { GPIO_52, GK_GPIO_UNDEFINED        }, \
            { GPIO_53, GK_GPIO_UNDEFINED        }, \
            { GPIO_54, GK_GPIO_UNDEFINED        }, \
            { GPIO_55, GK_GPIO_UNDEFINED        }, \
            { GPIO_56, GK_GPIO_UNDEFINED        }, \
            { GPIO_57, GK_GPIO_UNDEFINED        }, \
            { GPIO_58, GK_GPIO_UNDEFINED        }, \
            { GPIO_59, GK_GPIO_UNDEFINED        }, \
            { GPIO_60, GK_GPIO_UNDEFINED        }, \
            { GPIO_61, GK_GPIO_I2C0_DATA        }, \
            { GPIO_62, GK_GPIO_SENSOR_RESET     }, \
            { GPIO_63, GK_GPIO_I2C0_CLK         }, \
            { GPIO_64, GK_GPIO_UNDEFINED        }, \
            { GPIO_65, GK_GPIO_UNDEFINED        }, \
            { GPIO_66, GK_GPIO_UNDEFINED        }, \
            { GPIO_67, GK_GPIO_UNDEFINED        }, \
            { GPIO_68, GK_GPIO_UNDEFINED        }, \
            { GPIO_69, GK_GPIO_UNDEFINED        }, \
            { GPIO_70, GK_GPIO_OUTPUT_0        }, \
            { GPIO_71, GK_GPIO_UNDEFINED        }, \
            { GPIO_72, GK_GPIO_UNDEFINED        }, \
            { GPIO_73, GK_GPIO_UNDEFINED        }, \
            { GPIO_74, GK_GPIO_UNDEFINED        }, \
            { GPIO_75, GK_GPIO_UNDEFINED        }, \
            { GPIO_76, GK_GPIO_UNDEFINED        }, \
            { GPIO_77, GK_GPIO_UNDEFINED        }, \
            { GPIO_78, GK_GPIO_UNDEFINED        }, \
            { GPIO_79, GK_GPIO_UNDEFINED        },
#endif /* CONFIG_MACH_GK_T26_R25_K */

#endif /* _GK_GPIO_T26_R25_H_ */

