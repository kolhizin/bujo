import numpy as np
import tensorflow as tf
import sys

class HTRModel:
    def __init__(self, charlist, img_size=(128, 32), text_len=32,
                 cnn_kernels = [5, 5, 3, 3, 3],
                 cnn_features = [1, 32, 64, 128, 128, 256],
                 cnn_pools = [(2,2), (2,2), (1,2), (1,2), (1,2)],
                 cnn_strides = None,
                 rnn_cells = [256, 256],
                 rnn_type = 'static', #'dynamic'
                 decoder = 'best-path',
                 model_path = '/htr-model/',
                 restore=False):
        self.chars_ = charlist
        self.restore_ = restore
        self.epochID_ = 0
        self.img_size_ = img_size
        self.text_len_ = text_len
        
        self.cnn_kernels_ = cnn_kernels
        self.cnn_features_ = cnn_features
        self.cnn_pools_ = cnn_pools
        self.cnn_strides_ = cnn_strides
        if cnn_pools is None and cnn_strides is None:
            raise Exception("Must specify at least one of `pools` and `strides`!")
        if cnn_pools is None:
            self.cnn_pools_ = cnn_strides
        if cnn_strides is None:
            self.cnn_strides_ = cnn_pools
        self.rnn_cells_ = rnn_cells
        self.model_path_ = model_path
        
        if decoder not in ('best-path', 'beam-search'):
            raise Exception("HTRModel: unknown decoder name `{}`. Expected `best-path` or `beam-search`".format(decoder))
        self.decoder_ = decoder
        
        tf.compat.v1.reset_default_graph()
        self.tf_is_train_ = tf.compat.v1.placeholder_with_default(False, shape=[], name='is_train')
        self.tf_in_images_ = tf.compat.v1.placeholder(tf.float32, shape=(None, self.img_size_[0], self.img_size_[1]), name='in_image')
        self.tf_seq_len_ = tf.compat.v1.placeholder(tf.int32, [None], name='in_length')

        self.tf_cnn_out_ = HTRModel.setupCNN_(self.tf_in_images_, self.tf_is_train_,
                    self.cnn_kernels_, self.cnn_features_, self.cnn_pools_, self.cnn_strides_)
        
        if rnn_type == 'static':
            self.tf_rnn_out_ = HTRModel.setupStaticRNN_(self.tf_cnn_out_, len(self.chars_), self.rnn_cells_)
        elif rnn_type == 'dynamic':
            self.tf_rnn_out_ = HTRModel.setupRNN_(self.tf_cnn_out_, len(self.chars_), self.rnn_cells_)
        else:
            raise ValueError("HTRModel: unexpected type of rnn!")
        self.setupCTC_()

        self.snap_id_ = 0
        self.trained_samples_ = 0
        self.tf_learning_rate_ = tf.compat.v1.placeholder(tf.float32, shape=[])
        self.tf_update_ops_ = tf.get_collection(tf.GraphKeys.UPDATE_OPS) 
        with tf.control_dependencies(self.tf_update_ops_):
            self.tf_optimizer_ = tf.train.RMSPropOptimizer(self.tf_learning_rate_).minimize(self.tf_loss_)

        (self.tf_session_, self.tf_saver_) = HTRModel.setupTF_(model_path)
        
    def setupCNN_(tf_input, tf_is_train, kernels, features, pools, strides, use_batch_norm=False):
        chk1 = len(kernels)+1 != len(features)
        chk2 = len(kernels) != len(pools)
        chk3 = len(pools) != len(strides)
        if chk1 or chk2 or chk3:
            print(len(kernels), len(pools), len(strides), len(features))
            raise Exception("HTRModel.setupCNN: lengths of arguments mismatch!")
            
        tf_cnn_input = tf.expand_dims(input=tf_input, axis=3)

        pool = tf_cnn_input
        for i in range(len(kernels)):
            kernel = tf.Variable(tf.truncated_normal([kernels[i], kernels[i], features[i], features[i + 1]], stddev=0.1))
            conv = tf.nn.conv2d(pool, kernel, padding='SAME',  strides=(1,1,1,1))
            conv_norm = conv
            if use_batch_norm:
                conv_norm = tf.layers.batch_normalization(conv, training=tf_is_train)
            relu = tf.nn.relu(conv_norm)
            pool = tf.nn.max_pool(relu, (1, pools[i][0], pools[i][1], 1), (1, strides[i][0], strides[i][1], 1), 'VALID')

        return pool


    def setupRNN_(tf_input, charnum, cell_sizes):
        rnn_input = tf.squeeze(tf_input, axis=[2])
        #default [256, 256]

        cells = [tf.contrib.rnn.LSTMCell(num_units=x, state_is_tuple=True) for x in cell_sizes]
        stacked = tf.contrib.rnn.MultiRNNCell(cells, state_is_tuple=True)

        # bidirectional RNN, BxTxF -> BxTx2H
        ((fw, bw), _) = tf.nn.bidirectional_dynamic_rnn(cell_fw=stacked, cell_bw=stacked, inputs=rnn_input, dtype=rnn_input.dtype)
        # BxTxH + BxTxH -> BxTx2H -> BxTx1X2H
        concat = tf.expand_dims(tf.concat([fw, bw], 2), 2)
        
        # project output to chars (including blank): BxTx1x2H -> BxTx1xC -> BxTxC
        kernel = tf.Variable(tf.truncated_normal([1, 1, cell_sizes[-1]*2, charnum + 1], stddev=0.1))
        return tf.squeeze(tf.nn.atrous_conv2d(value=concat, filters=kernel, rate=1, padding='SAME'), axis=[2], name='out_rnn')
    
    
    def setupStaticRNN_(tf_input, charnum, cell_sizes):
        rnn_input = tf.squeeze(tf_input, axis=[2])

        cells = [tf.contrib.rnn.LSTMCell(num_units=x, state_is_tuple=True) for x in cell_sizes]
        stacked = tf.contrib.rnn.MultiRNNCell(cells, state_is_tuple=True)
        
        res = tf.keras.layers.RNN(stacked, unroll=True, return_sequences=True)(rnn_input)
        concat = tf.expand_dims(res, 2)

        kernel = tf.Variable(tf.truncated_normal([1, 1, cell_sizes[-1], charnum + 1], stddev=0.1))
        return tf.squeeze(tf.nn.atrous_conv2d(value=concat, filters=kernel, rate=1, padding='SAME'), axis=[2], name='out_rnn')
    
    def setupCTC_(self):
        self.tf_ctc_in_ = tf.transpose(self.tf_rnn_out_, [1, 0, 2]) # BxTxC -> TxBxC
        # ground truth text as sparse tensor
        self.tf_ctc_gt_ = tf.SparseTensor(tf.compat.v1.placeholder(tf.int64, shape=[None, 2]),
                    tf.compat.v1.placeholder(tf.int32, [None]),
                    tf.compat.v1.placeholder(tf.int64, [2]))

        # calc loss for batch
        self.tf_loss_ = tf.reduce_mean(tf.nn.ctc_loss(labels=self.tf_ctc_gt_, inputs=self.tf_ctc_in_, sequence_length=self.tf_seq_len_, ctc_merge_repeated=True))

        # calc loss for each element to compute label probability
        self.tf_ctc_in_saved_ = tf.compat.v1.placeholder(tf.float32, shape=[self.text_len_, None, len(self.chars_) + 1])
        self.tf_loss_per_elem_ = tf.nn.ctc_loss(labels=self.tf_ctc_gt_, inputs=self.tf_ctc_in_saved_, sequence_length=self.tf_seq_len_, ctc_merge_repeated=True)

        if self.decoder_ == 'best-path':
            self.tf_decoder_ = tf.nn.ctc_greedy_decoder(inputs=self.tf_ctc_in_, sequence_length=self.tf_seq_len_)
        elif self.decoder_ == 'beam-search':
            self.tf_decoder_ = tf.nn.ctc_beam_search_decoder(inputs=self.tf_ctc_in_, sequence_length=self.tf_seq_len_, beam_width=50, merge_repeated=False)
    
    def setupTF_(model_path, max_to_keep=1):
        print('Python: {}; TF: {}'.format(sys.version, tf.__version__))
        sess = tf.compat.v1.Session()
        saver = tf.compat.v1.train.Saver(max_to_keep=max_to_keep)
        latest_snapshot = tf.train.latest_checkpoint(model_path)
        if latest_snapshot:
            print('Starting hot: {}'.format(latest_snapshot))
            saver.restore(sess, latest_snapshot)
        else:
            print('Starting cold')
            sess.run(tf.global_variables_initializer())

        return (sess,saver)

    def getTFLiteBuffer(self):
        converter = tf.lite.TFLiteConverter.from_session(self.tf_session_, [self.tf_in_images_], [self.tf_rnn_out_])
        return converter.convert()

    def getFrozenGraph(self, clear_devices=True):
        graph = self.tf_session_.graph
        output_names = [self.tf_decoder_[1].name.split(':')[0], 'out_rnn']
        with graph.as_default():
            freeze_var_names = [v.op.name for v in tf.global_variables()]
            input_graph_def = graph.as_graph_def()
            if clear_devices:
                for node in input_graph_def.node:
                    node.device = ""
            return tf.graph_util.convert_variables_to_constants(self.tf_session_, input_graph_def, output_names, freeze_var_names)
        
    
    def encodeLabels(self, texts):
        indices = []
        values = []
        shape = [len(texts), max(len(x) for x in texts)] # last entry must be max(labelList[i])

        for (i, text) in enumerate(texts):
            encoded_text = [self.chars_.index(c) for c in text]
            for (j, label) in enumerate(encoded_text):
                indices.append([i, j])
                values.append(label)
        return (indices, values, shape)


    def decodeOutput(self, ctc_output, batch_size):
        encoded_labels = [[] for i in range(batch_size)]

        decoded=ctc_output[0][0] 
        # go over all indices and save mapping: batch -> values
        for (k, (i, j)) in enumerate(decoded.indices):
            label = decoded.values[k]
            encoded_labels[i].append(label)

        return [''.join([self.chars_[c] for c in x]) for x in encoded_labels]
    
    def getLearningRate(self):
        return 0.01 if self.trained_samples_ < 1e4 else (0.001 if self.trained_samples_ < 1e5 else 0.0001)


    def trainBatch(self, imgs, texts):
        batch_size = len(imgs)
        gt_sparse = self.encodeLabels(texts)
        rate =  self.getLearningRate()
        evalList = [self.tf_optimizer_, self.tf_loss_]
        feedDict = {self.tf_in_images_ : imgs,
                    self.tf_ctc_gt_ : gt_sparse,
                    self.tf_seq_len_ : [self.text_len_] * batch_size,
                    self.tf_learning_rate_ : rate,
                    self.tf_is_train_: True}
        (_, lossVal) = self.tf_session_.run(evalList, feedDict)
        self.trained_samples_ += batch_size
        return lossVal
    
    def validBatch(self, imgs, texts):
        batch_size = len(imgs)
        gt_sparse = self.encodeLabels(texts)
        evalList = [self.tf_decoder_, self.tf_loss_]
        feedDict = {self.tf_in_images_ : imgs,
                    self.tf_ctc_gt_ : gt_sparse,
                    self.tf_seq_len_ : [self.text_len_] * batch_size,
                    self.tf_is_train_: False}
        (evalRes, lossVal) = self.tf_session_.run(evalList, feedDict)
        return self.decodeOutput(evalRes, batch_size), lossVal
    
    def inferBatch(self, imgs):
        batch_size = len(imgs)
        evalList = [self.tf_decoder_]
        feedDict = {self.tf_in_images_ : imgs,
                    self.tf_seq_len_ : [self.text_len_] * batch_size,
                    self.tf_is_train_: False}
        evalRes = self.tf_session_.run(evalList, feedDict)
        return self.decodeOutput(evalRes[0], batch_size)

    def inferRaw(self, imgs):
        batch_size = len(imgs)
        evalList = [self.tf_out_rnn_]
        feedDict = {self.tf_in_images_ : imgs,
                    self.tf_seq_len_ : [self.text_len_] * batch_size,
                    self.tf_is_train_: False}
        evalRes = self.tf_session_.run(evalList, feedDict)
        return evalRes[0]

    def save(self):
        self.snap_id_ += 1
        self.tf_saver_.save(self.tf_session_, self.model_path_ + 'snapshot', global_step=self.snap_id_)