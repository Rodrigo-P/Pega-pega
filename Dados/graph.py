# -*- coding: UTF-8 -*-
import matplotlib.pyplot as plt
import pandas as pd
import glob

lista = glob.glob('./*.txt')

for arq in lista:
	df = pd.read_csv(arq, sep=" ", header=None, names=["ger", "a", "b", "c", "d", "e", "f"])

	#print(df.ger)

	df.plot(title=arq[2:],x='ger')

	plt.show()