import matplotlib.pyplot as plt
import numpy as np
import csv
import sys

# 获取命令行参数
params_file = sys.argv[1]
data_file = sys.argv[2]

# 读取CSV文件中的参数
with open(params_file, 'r') as f:
    reader = csv.reader(f)
    params = next(reader)  # 获取第一行数据
    a, b = map(float, params)  # 将参数转换为浮点数

# 读取样本数据
with open(data_file, 'r') as f:
    reader = csv.reader(f)
    sample_data = [float(row[0]) for row in reader]  # 将样本数据转换为浮点数

# 创建x坐标
x = np.linspace(0, len(sample_data) - 1, len(sample_data))  # x坐标从0开始，到样本数据的长度结束

# 计算y坐标
y = a * x + b

# 绘制线性回归图形
plt.plot(x, y, label='Linear Regression')

# 绘制样本数据
plt.plot(x, sample_data, 'ro', label='Sample Data')

plt.title('Linear Regression')
plt.xlabel('x')
plt.ylabel('y')
plt.grid(True)
plt.xlim(left=0)  # x轴只显示非负实数区域
plt.ylim(bottom=0)  # y轴只显示非负实数区域
plt.show(block=False)  # 非阻塞显示

plt.pause(3.5)  # 暂停3.5秒
plt.close()  # 关闭显示窗口
