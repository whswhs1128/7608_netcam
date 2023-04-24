#ifndef _DEBUG_H_
#define _DEBUG_H_

/*
\e 或 \033 来输出Esc符号
设置颜色的格式： \e[背景色;前景色;高亮m
\033[背景色;前景色;高亮m
恢复默认为       \e[0m
其中背景色可以被以下数字替换

第一个参数：
0 透明（使用终端颜色）,1 高亮 40 黑, 41 红, 42 绿, 43 黄, 44 蓝 45 紫, 46 青 ; 我们选了这里的 1 高亮
绿, 47白（灰）

第二个参数：
前景色（也就是文本的颜色）可以被以下数字替换
30 黑 31 红, 32 绿, 33 黄, 34 蓝, 35 紫, 36 青绿, 37 白（灰）

第三个参数：
高亮是1，不高亮是0

第四个参数为m:
注意m后面紧跟字符串。

echo -e "\e[1;30m skyapp exist \e[0m" //黑色
echo -e "\e[1;31m skyapp exist \e[0m" //红色
echo -e "\e[1;32m skyapp exist \e[0m" //绿色
echo -e "\e[1;33m skyapp exist \e[0m" //黄色
echo -e "\e[1;34m skyapp exist \e[0m" //蓝色
echo -e "\e[1;35m skyapp exist \e[0m" //紫色
echo -e "\e[1;36m skyapp exist \e[0m" //青绿
echo -e "\e[1;37m skyapp exist \e[0m" //白（灰）
*/

#define NONE                 "\e[0m"
#define BLACK                "\e[0;30m"
#define L_BLACK              "\e[1;30m"
#define RED                  "\e[0;31m"
#define L_RED                "\e[1;31m"
#define GREEN                "\e[0;32m"
#define L_GREEN              "\e[1;32m"
#define BROWN                "\e[0;33m"
#define YELLOW               "\e[1;33m"
#define BLUE                 "\e[0;34m"
#define L_BLUE               "\e[1;34m"
#define PURPLE               "\e[0;35m"
#define L_PURPLE             "\e[1;35m" //1表示高亮, 跟下面的BOLD效果是一样的
#define CYAN                 "\e[0;36m"
#define L_CYAN               "\e[1;36m"
#define GRAY                 "\e[0;37m"
#define WHITE                "\e[1;37m"

#define COLOR_TAIL           "\e[0m" //恢复

#define BOLD                 "\e[1m"
#define UNDERLINE            "\e[4m"
#define BLINK                "\e[5m"
#define REVERSE              "\e[7m"
#define HIDE                 "\e[8m"
#define CLEAR                "\e[2J"
#define CLRLINE              "\r\e[K" //or "\e[1K\r"

//这里是glnk相关的打印
#define __GLNK_DEBUG__  
#ifdef  __GLNK_DEBUG__  
#define GDEBUG(format,...) printf(YELLOW  "%s:%03d " format "\n" COLOR_TAIL, __FUNCTION__,  __LINE__, ##__VA_ARGS__)  
#else  
#define GDEBUG(format,...)  
#endif  

//这里是cc的打印
#define __DEBUG__  
#ifdef  __DEBUG__
#define DEBUG(format,...) printf(L_PURPLE "Fnx_CCProcess, %s:%03d " format "\n" COLOR_TAIL, __FUNCTION__,  __LINE__, ##__VA_ARGS__)  
#else  
#define DEBUG(format,...)  
#endif  

#endif