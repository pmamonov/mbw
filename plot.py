#!/usr/bin/env python3

import sys
import matplotlib
import numpy as np

def genplot(log):
	with open(log) as f:
		lbl = f.readline().split()[2:]
	d = np.loadtxt(log)

	for i in range(1, d.shape[1]):
		plt.plot(d[:, 0], d[:, i], label=lbl[i - 1])

	plt.xscale("log")
	plt.xticks(d[:,0], ["%d" % x for x in d[:, 0]])
	plt.xlabel("kB")

	plt.ylabel("MB/s")
	plt.ylim(bottom=0)

	plt.grid()
	plt.legend()
	plt.title(log.split(".")[0])

if __name__ == "__main__":
	if len(sys.argv) < 3:
		print("USAGE: %s file.log file.png")
		sys.exit(1)
	log = sys.argv[1]
	png = sys.argv[2]

	matplotlib.use("agg")
	import matplotlib.pyplot as plt

	plt.figure(figsize=(15, 6))
	genplot(log)
	plt.savefig(png)
