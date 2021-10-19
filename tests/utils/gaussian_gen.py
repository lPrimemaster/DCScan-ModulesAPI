import math
import argparse as ap
import numpy as np
import operator
import functools as ft

def gaussian(mu, sigma, x):
	return (1 / (math.sqrt(2 * math.pi) * sigma)) * math.exp(-0.5 * ((x - mu) / sigma) ** 2)

def generateGaussian(mu, sigma, chunk, h):
	'''Generate a gaussian from ]u - 3s, u + 3s['''
	sig3 = 3 * sigma
	a = int(mu - sig3)
	b = int(mu + sig3)
	size = b - a

	return (size, [h * gaussian(mu, sigma, x) for x in np.linspace(a, b, num=size, dtype=float)])


if __name__ == '__main__':
	p = ap.ArgumentParser(description='Generate gaussian peaks randomly.')
	p.add_argument('-o', '--sdev', dest='sdev', type=int)
	p.add_argument('-r', '--compactness', dest='compactness', type=int)
	p.add_argument('-c', '--chunkSize', dest='chunkSize', type=int)
	p.add_argument('-n', '--numChunks', dest='numChunks', type=int)

	args = p.parse_args()

	for c in range(args.numChunks):
		# Pick at most r max peaks
		peaks = np.sort(np.random.randint(args.chunkSize - 50, size=(np.random.randint(1, args.compactness + 1, size=(1,))[0],)))

		out = []
		res = []

		for p in peaks:

			while(len(out) < p - 3 * args.sdev):
				out.append(0.0)

			size, vals = generateGaussian(p, args.sdev, args.chunkSize, np.random.randint(5, 10 + 1, size=(1,))[0])

			if(p < 30):
				res.append(p + 30)
			else:
				res.append(p)

			for i in vals:
				out.append(i)

		while(len(out) < args.chunkSize):
			out.append(0.0)

		if(len(out) > args.chunkSize):
			out = out[:args.chunkSize]
			
		res.append(len(peaks))

		print('Generating chunk #' + str(c) + '...', res)

		with open('c_' + str(c) + '.txt', 'w') as f:
			f.write('\n'.join(list(map(str, out))))
			f.write('\n')
			f.write('\n'.join(list(map(str, res))))

	print('\t\t     fmt=[p0, p1, ..., pN, counts]\n\n')
	print('\t\t     Chunk Size:', args.chunkSize)
	print('\t\t     Num Chunks:', args.numChunks)
	print('\t\t         StdDev:', args.sdev)
	print('\t\tMax Compactness:', args.compactness)