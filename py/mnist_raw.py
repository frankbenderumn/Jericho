
import numpy as np
import matplotlib.pyplot as plt

def convert(img_file, label_file, txt_file, n_images):
    print("\nOpening binary pixels and label files")
    lbl_f = open(label_file, "rb")
    img_f = open(img_file, "rb")
    print("Opening destination file ")
    txt_f = open(txt_file, "w")

    print("Discarding binary pixel and label headers")
    img_f.read(16) # discard header info
    lbl_f.read(8) # discard header info

    print("\nReading binary files, writing to text file")
    print("Format: 784 pixels then labels, tab delimited")

    for i in range(n_images):
        lbl = ord(lbl_f.read(1))
        txt_f.write(str(lbl) + ",")
        for j in range(783):
            val = ord(img_f.read(1))
            txt_f.write(str(val) + ",")
        txt_f.write(str(ord(img_f.read(1))) + "\n")
    img_f.close(); txt_f.close(); lbl_f.close()
    print("\nDone")

def display_from_file(txt_file, idx):
    all_data = np.loadtxt(txt_file, delimiter=",",
    usecols=range(0,785), dtype=np.int64)
    x_data = all_data[:,1:785]
    y_data = all_data[:,0]

    label = y_data[idx]
    pixels = x_data[idx]
    pixels = pixels.reshape((28,28))

    print("Label is:",label)
    for i in range(28):
        for j in range(28):
            print("%3d" % pixels[i,j], end="")
            print(" ", end="")
        print("")

    plt.tight_layout()
    plt.imshow(pixels, cmap=plt.get_cmap('gray_r'))
    plt.show()

def main():
    n_images = 1000
    print("Creating %d MNIST train images from binary files " % n_images)
    convert("./py/data/MNIST/raw/train-images-idx3-ubyte.bin",
            "./py/data/MNIST/raw/train-labels-idx1-ubyte.bin",
            "./py/data/MNIST/train.txt",
            60000)
    convert("./py/data/MNIST/raw/t10k-images-idx3-ubyte.bin",
            "./py/data/MNIST/raw/t10k-labels-idx1-ubyte.bin",
            "./py/data/MNIST/test.txt",
            10000)
    print("Showing train image[0]:")
    img_file = "./py/data/MNIST/train.txt"
    display_from_file(img_file, idx=26)

main()