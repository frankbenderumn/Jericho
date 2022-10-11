import os
import numpy as np
import sys
import tensorflow as tf
print("TensorFlow version:", tf.__version__)

from tensorflow.keras.layers import Dense, Flatten, Conv2D
from tensorflow.keras import Model
mnist = tf.keras.datasets.mnist

def print_statistics(i, x_test_pi, x_train_pi, nb_labels, y_train_pi):
    print('Party_', i)
    print('nb_x_train: ', np.shape(x_train_pi),
          'nb_x_test: ', np.shape(x_test_pi))
    for l in range(nb_labels):
        print('* Label ', l, ' samples: ', (y_train_pi == l).sum())

def save_mnist_party_data(nb_dp_per_party, should_stratify, party_folder, dataset_folder):
    """
    Saves MNIST party data
    :param nb_dp_per_party: the number of data points each party should have
    :type nb_dp_per_party: `list[int]`
    :param should_stratify: True if data should be assigned proportional to source class distributions
    :type should_stratify: `bool`
    :param party_folder: folder to save party data
    :type party_folder: `str`
    :param dataset_folder: folder to save dataset
    :type data_path: `str`
    :param dataset_folder: folder to save dataset
    :type dataset_folder: `str`
    """
    if not os.path.exists(dataset_folder):
        os.makedirs(dataset_folder)
    # (x_train, y_train), (x_test, y_test) = load_mnist(download_dir=dataset_folder)
    (x_train, y_train), (x_test, y_test) = mnist.load_data()
    labels, train_counts = np.unique(y_train, return_counts=True)
    te_labels, test_counts = np.unique(y_test, return_counts=True)
    if np.all(np.isin(labels, te_labels)):
        print("Warning: test set and train set contain different labels")
    print("Number of clients: ",nb_dp_per_party)
    num_train = np.shape(y_train)[0]
    num_test = np.shape(y_test)[0]
    num_labels = np.shape(np.unique(y_test))[0]
    nb_parties = len(nb_dp_per_party)

    if should_stratify:
        # Sample according to source label distribution
        train_probs = {
            label: train_counts[label] / float(num_train) for label in labels}
        test_probs = {label: test_counts[label] /
                      float(num_test) for label in te_labels}
    else:
        # Sample uniformly
        train_probs = {label: 1.0 / len(labels) for label in labels}
        test_probs = {label: 1.0 / len(te_labels) for label in te_labels}

    for idx, dp in enumerate(nb_dp_per_party):
        train_p = np.array([train_probs[y_train[idx]]
                            for idx in range(num_train)])
        train_p /= np.sum(train_p)
        train_indices = np.random.choice(num_train, dp, p=train_p)
        test_p = np.array([test_probs[y_test[idx]] for idx in range(num_test)])
        test_p /= np.sum(test_p)

        # Split test evenly
        test_indices = np.random.choice(
            num_test, int(num_test / nb_parties), p=test_p)

        x_train_pi = x_train[train_indices]
        y_train_pi = y_train[train_indices]
        x_test_pi = x_test[test_indices]
        y_test_pi = y_test[test_indices]

        # Now put it all in an npz
        name_file = 'data_party' + str(idx) + '.npz'
        name_file = os.path.join(party_folder, name_file)
        np.savez(name_file, x_train=x_train_pi, y_train=y_train_pi,
                 x_test=x_test_pi, y_test=y_test_pi)

        print_statistics(idx, x_test_pi, x_train_pi, num_labels, y_train_pi)

        print('Finished! :) Data saved in ', party_folder)

args = sys.argv
print('Number of arguments:', len(args), 'arguments.')
if (len(args) < 2):
    sys.exit("Invalid number of arguments")
print('Argument List:', str(args))

save_mnist_party_data(args[1], False, "./py/mnist", "./py/mnist-client");


