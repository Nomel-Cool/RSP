import matplotlib.pyplot as plt
import numpy as np
import csv
import time

# 读取CSV文件中的参数
with open('regression_params.csv', 'r') as f:
    reader = csv.reader(f)
    params = next(reader)  # 获取第一行数据
    a, b = map(float, params)  # 将参数转换为浮点数

# 创建x坐标
x = np.linspace(0, 10, 400)  # x坐标从0开始

# 计算y坐标
y = a * x + b

# 绘制图形
plt.plot(x, y)
plt.title('Linear Regression')
plt.xlabel('x')
plt.ylabel('y')
plt.grid(True)
plt.xlim(left=0)  # x轴只显示非负实数区域
plt.ylim(bottom=0)  # y轴只显示非负实数区域
plt.show(block=False)  # 非阻塞显示

time.sleep(1.5)  # 等待1.5秒
plt.close()  # 关闭显示窗口
