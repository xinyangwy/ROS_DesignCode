﻿README for Linux SDK
-----------------------

bin：
|-- msc
    |-- msc.cfg（作用：msc调试、生成msc日志）
|-- wav（符合标准的音频文件样例）
|-- 示例程序可执行文件（samples目录下对应示例生成的可执行文件）
|-- gm_continuous_digit.abnf（abnf语法样例）
|-- userwords.txt（用户词表样例）

doc：
|-- iFlytek MSC Reference Manual（API文档，HTML格式）
|-- MSC Novice Manual for Windows.pdf（MSC新手指南）
|-- Grammar Development Guidelines（语音识别语法规范）
|-- Open Semantic Platform API Documents（语义开放平台API文档）

include：调用SDK所需头文件

libs：
|-- x86
	|-- libmsc.so（32位动态库）
|-- x64
	|-- libmsc.so（64位动态库）

samples：
|-- asr_sample（语音识别示例）
	|-- asr_sample.c
	|-- Makefile
	|-- 32bit_make.sh
	|-- 64bit_make.sh
|-- iat_sample（语音听写示例）
	|-- iat_sample.c
	|-- Makefile
	|-- 32bit_make.sh
	|-- 64bit_make.sh
|-- ise_sample（语音评测示例）
	|-- ise_sample.c
	|-- Makefile
	|-- 32bit_make.sh
	|-- 64bit_make.sh
|-- tts_sample（语音合成示例）
	|-- tts_sample.c
	|-- Makefile
	|-- 32bit_make.sh
	|-- 64bit_make.sh
|-- sch_speak_sample（语音语义示例）
	|-- sch_speak_sample.c
	|-- Makefile
	|-- 32bit_make.sh
	|-- 64bit_make.sh
|-- sch_text_sample（文本语义示例）
	|-- sch_text_sample.c
	|-- Makefile
	|-- 32bit_make.sh
	|-- 64bit_make.sh
|-- iat_record_sample（录音听写示例）
	|-- iat_record_sample.c
	|-- Makefile
	|-- 32bit_make.sh
	|-- 64bit_make.sh
|-- sch_translate_sample（语音翻译示例）
	|-- sch_translate_sample.c
	|-- json.c
	|-- json.h
	|-- Makefile
	|-- 32bit_make.sh
	|-- 64bit_make.sh