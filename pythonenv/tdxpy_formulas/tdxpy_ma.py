#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
@filename: tdxpy_ma.py
@brief: 通达信移动平均线(MA)计算模块
@details:
    本模块提供通达信兼容的移动平均线计算公式实现，支持多周期MA计算。
    采用Python实现通达信公式接口，与tdxpy框架集成使用。

    主要特性:
    1. 支持简单移动平均(SMA)计算
    2. 支持多周期参数配置
    3. 符合通达信公式接口规范
    4. 数据长度自动处理（由通达信软件传入）

@author: 码上工坊
@copyright: Copyright (c) 2026-2030 码上工坊
@license: MIT License (详见项目根目录LICENSE文件)
@version: 0.1.0
@date: 2026-01-10

@dependencies:
    - Python >= 3.7
    - 标准库: math, typing

@usage:
    # 在通达信公式管理器中调用:
    TDXDLLn(n, C, 0, 0);

@changelog:
    2026-01-10:
        - 版本: 1.0.0
        - 作者: 码上工坊
        - 描述: 初始版本，实现基本移动平均功能
"""

from typing import List


def tdxpy_ma(
    function_id: int,
    data_length: int,
    input_a: List[float],
    input_b: List[float],
    input_c: List[float],
    user_params: str,
) -> List[float]:
    """
    通达信Python公式移动平均线入口函数（示例函数）

    参数:
        function_id: 函数ID (对应配置文件中的id)
        data_length: 数据长度
        input_a: 输入数据A (通常是收盘价)
        input_b: 输入数据B (通常是最高价)
        input_c: 输入数据C (通常是最低价)
        user_params: 用户参数字符串 (如"5,10,20")

    返回:
        list: 计算结果列表，长度必须等于data_length（返回给通达信软件的数据）
    """
    # 解析用户参数
    periods = (
        [int(p.strip()) for p in user_params.split(",")] if user_params else [5, 10, 20]
    )

    # 计算第一个周期的移动平均
    period = periods[0]
    result = []

    for i in range(data_length):
        if i < period - 1:
            result.append(0.0)
        else:
            # 计算简单移动平均
            sum_val = sum(input_a[i - period + 1 : i + 1])
            result.append(sum_val / period)

    return result


# 测试代码
if __name__ == "__main__":
    # 测试数据
    test_prices = [i * 1.0 for i in range(1, 21)]  # 1.0到20.0

    print("=== TDXPy移动平均线测试 ===")
    print(f"测试数据: {test_prices}")
    print(f"数据长度: {len(test_prices)}")
    print()

    # 测试通达信接口函数
    result = tdxpy_ma(1, len(test_prices), test_prices, [], [], "5")
    print(f"通达信接口SMA(5): {result}")
    print()
