package com.kolhizin.detectbujo;

import android.app.Activity;
import android.content.res.AssetFileDescriptor;
import android.graphics.Bitmap;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Comparator;
import java.util.LinkedList;

import org.tensorflow.lite.Interpreter;
import org.tensorflow.lite.Tensor;

public class Classifier {
    public class CharResult {
        float [] probs;
        char [] chars;
    }
    public class StringResult {
        String string;
        float prob;
    }

    private static float [] convertProbs(float [] arr){
        double [] exps = new double[arr.length];
        double sum = 0.0;
        for(int i = 0; i < arr.length; i++){
            exps[i] = Math.exp(arr[i]);
            sum += exps[i];
        }
        float [] res = new float[arr.length];
        for(int i = 0; i < arr.length; i++){
            res[i] = (float)(exps[i] / sum);
        }
        return res;
    }
    private static Integer [] getSortedIndices(float [] arr){
        class ArrayIndexComparator implements Comparator<Integer>
        {
            private final float[] array_;

            public ArrayIndexComparator(float[] array) {
                array_ = array;
            }

            public Integer[] createIndexArray() {
                Integer[] indexes = new Integer[array_.length];
                for (int i = 0; i < array_.length; i++) {
                    indexes[i] = i; // Autoboxing
                }
                return indexes;
            }

            @Override
            public int compare(Integer index1, Integer index2) {
                return -Float.compare(array_[index1], array_[index2]);
            }
        }
        ArrayIndexComparator comparator = new ArrayIndexComparator(arr);
        Integer [] indices = comparator.createIndexArray();
        Arrays.sort(indices, comparator);
        return indices;
    }

    public Interpreter tflite_;
    private Activity activity_;
    private String chars_;
    private ByteBuffer inputData_ = null;
    private float[][][] outputData_ = null;

    private float [][][] buffers_ = null;
    private int [] bufferRows_ = null;
    private int [] bufferCols_ = null;
    private int activeBuffer_ = 0;

    private int dim_batch_size;
    private int dim_in_rows;
    private int dim_in_cols;
    private int dim_out_steps;
    private int dim_out_chars;

    public Classifier(Activity activity, String modelPath, String charsPath) throws IOException, RuntimeException {
        activity_ = activity;
        chars_ = loadChars(charsPath);
        tflite_ = new Interpreter(loadModelFile(modelPath), null);
        updateDims_();
        if(chars_.length() + 1 != dim_out_chars)
            throw new RuntimeException("Classifier: provided charset does not match model output!");

        updateBuffers_(dim_in_rows, dim_in_cols);

        inputData_ = ByteBuffer.allocateDirect(dim_batch_size * dim_in_rows * dim_in_cols * 4);
        inputData_.order(ByteOrder.nativeOrder());
        outputData_ = new float[dim_batch_size][dim_out_steps][dim_out_chars];
    }

    private void updateDims_() throws RuntimeException{
        Tensor in = tflite_.getInputTensor(0);
        Tensor out = tflite_.getOutputTensor(0);
        if((in.numDimensions() <= 1) || (in.numDimensions() >= 4))
            throw new RuntimeException("Classifier: unexpected model-input dimensions!");
        if((in.numDimensions() == 3) && (in.shape()[0] != 1))
            throw new RuntimeException("Classifier: unexpected model-input dimensions -- batch_size greater 1!");
        if(out.numDimensions() != in.numDimensions())
            throw new RuntimeException("Classifier: dimensionality mismatch in input/output!");
        if((out.numDimensions() == 3) && (out.shape()[0] != 1))
            throw new RuntimeException("Classifier: unexpected model-output dimensions -- batch_size greater 1!");
        if(in.dataType().name() != "FLOAT32")
            throw new RuntimeException("Classifier: unexpected model-input data type -- " + in.dataType().name());
        if(out.dataType().name() != "FLOAT32")
            throw new RuntimeException("Classifier: unexpected model-output data type -- " + out.dataType().name());

        int row_id = 1; //taking cols because transpose right before inference
        int col_id = 0; //taking rows because transpose right before inference
        if(in.numDimensions() == 3){
            row_id = 2;
            col_id = 1;
        }

        int steps_id = 0;
        int chars_id = 1;
        if(in.numDimensions() == 3){
            steps_id = 1;
            chars_id = 2;
        }
        dim_batch_size = 1;
        dim_in_rows = in.shape()[row_id];
        dim_in_cols = in.shape()[col_id];
        dim_out_steps = out.shape()[steps_id];
        dim_out_chars = out.shape()[chars_id];
    }

    private int nextBuffer_(){
        if(activeBuffer_ + 1 >= buffers_.length){
            activeBuffer_ = 0;
        }else{
            activeBuffer_ += 1;
        }
        return activeBuffer_;
    }

    private void updateBuffers_(int rows, int cols){
        int dim = Math.max(rows, cols);
        if(buffers_ == null){
            buffers_ = new float[2][dim*4][dim*4];
            bufferRows_ = new int [2];
            bufferCols_ = new int [2];
        }else if((buffers_[0].length < dim) || (buffers_[0][0].length < dim))
            growBuffers_(dim, dim);
    }

    private void growBuffers_(int rows, int cols){
        int nr = Math.max(rows, buffers_[0].length);
        int nc = Math.max(cols, buffers_[0][0].length);
        int nb = buffers_.length;
        float [][][] res = new float[nb][nr][nc];
        for(int i = 0; i < nb; i++){
            for(int j = 0; j < buffers_[i].length; j++){
                for(int k = 0; k < buffers_[i][j].length; k++){
                    res[i][j][k] = buffers_[i][j][k];
                }
            }
        }
        buffers_ = res;
    }

    private MappedByteBuffer loadModelFile(String modelPath) throws IOException {
        AssetFileDescriptor fileDescriptor = activity_.getAssets().openFd(modelPath);
        FileInputStream inputStream = new FileInputStream(fileDescriptor.getFileDescriptor());
        FileChannel fileChannel = inputStream.getChannel();
        long startOffset = fileDescriptor.getStartOffset();
        long declaredLength = fileDescriptor.getDeclaredLength();
        return fileChannel.map(FileChannel.MapMode.READ_ONLY, startOffset, declaredLength);
    }

    private String loadChars(String fname) throws IOException {
        InputStreamReader is = new InputStreamReader(activity_.getAssets().open(fname), StandardCharsets.UTF_16);
        BufferedReader reader = new BufferedReader(is);
        return reader.readLine();
    }

    private void setInput(float [][] input) throws RuntimeException{
        if (inputData_ == null)
            throw new RuntimeException("Internal input buffer is not initialized!");
        if (input.length != dim_in_cols)
            throw new RuntimeException("Input row number does not match expected by model!");
        if (input[0].length != dim_in_rows)
            throw new RuntimeException("Input col number does not match expected by model!");
        inputData_.rewind();
        for (int i = 0; i < dim_in_cols; i++) {
            for (int j = 0; j < dim_in_rows; j++) {
                inputData_.putFloat(input[i][j]);
            }
        }
    }

    private void setInput() throws RuntimeException{
        if (inputData_ == null)
            throw new RuntimeException("Internal input buffer is not initialized!");
        if (bufferRows_[activeBuffer_] != dim_in_cols)
            throw new RuntimeException("Input row number does not match expected by model!");
        if (bufferCols_[activeBuffer_] != dim_in_rows)
            throw new RuntimeException("Input col number does not match expected by model!");
        inputData_.rewind();
        for (int i = 0; i < bufferRows_[activeBuffer_]; i++) {
            for(int j = 0; j < bufferCols_[activeBuffer_]; j++){
                inputData_.putFloat(buffers_[activeBuffer_][i][j]);
            }
        }
    }

    private void runInference(){
        tflite_.run(inputData_, outputData_);
    }

    public float [][] detect(float [][] input){
        setInput(input);
        runInference();
        return outputData_[0];
    }
    public Bitmap preprocess(Bitmap src, float cutoff) throws Exception{
        applyTransforms(src, cutoff);
        transformTranspose();
        transformMinMax();
        return arrayToBitmap_(buffers_[activeBuffer_], bufferRows_[activeBuffer_], bufferCols_[activeBuffer_]);
    }
    public String detect(Bitmap src, float cutoff) throws Exception{
        applyTransforms(src, cutoff);
        setInput();
        runInference();
        return decodeGreedy(outputData_[0]);
    }

    public CharResult[] detect(Bitmap src, float cutoff, int topK) throws Exception{
        applyTransforms(src, cutoff);
        setInput();
        runInference();
        return decodeTopK(outputData_[0], topK);
    }
    public StringResult[] detect(Bitmap src, float cutoff, int numChanges, float optionCutoff) throws Exception{
        applyTransforms(src, cutoff);
        setInput();
        runInference();
        return decodeBestK(outputData_[0], numChanges, optionCutoff);
    }

    public float [][] transformInput(Bitmap src, float cutoff) throws Exception{
        applyTransforms(src, cutoff);
        float [][] res = new float[bufferRows_[activeBuffer_]][bufferCols_[activeBuffer_]];
        for(int i = 0; i < bufferRows_[activeBuffer_]; i++){
            for(int j = 0; j < bufferCols_[activeBuffer_]; j++){
                res[i][j] = buffers_[activeBuffer_][i][j];
            }
        }
        return res;
    }

    private CharResult[]decodeTopK(float [][] encoded, int k){
        CharResult[] res = new CharResult[encoded.length];
        for(int i = 0; i < encoded.length; i++){
            res[i] = new CharResult();
            res[i].probs = new float[k];
            res[i].chars = new char[k];
            float [] probs = convertProbs(encoded[i]);
            Integer[] inds = getSortedIndices(probs);
            for(int j = 0; j < k; j++){
                res[i].probs[j] = probs[inds[j]];
                if(inds[j] == chars_.length())
                    res[i].chars[j] = 0;
                else
                    res[i].chars[j] = chars_.charAt(inds[j]);
            }
        }
        return res;
    }

    private StringResult []decodeBestK(float [][] encoded, int numChanges, float probCutoff){
        //1 sort encoded result
        float [][] probs = new float[encoded.length][encoded[0].length];
        int [][] ids = new int[encoded.length][encoded[0].length];
        for(int i = 0; i < encoded.length; i++){
            float [] probs0 = convertProbs(encoded[i]);
            Integer[] inds0 = getSortedIndices(probs0);
            for(int j = 0; j < inds0.length; j++){
                ids[i][j] = inds0[j];
                probs[i][j] = probs0[inds0[j]];
            }
        }

        //2 get substitute priorities
        class ProbId {
            float prob;
            int i, j;
        }
        class FullSubstitute{
            float prob;
            ProbId [] subs;
        }
        class ProbIdComparator implements Comparator<ProbId>
        {
            public ProbIdComparator(){}
            @Override
            public int compare(ProbId p1, ProbId p2) {
                return -Float.compare(p1.prob, p2.prob);
            }
        }
        class StringResultComparator implements Comparator<Classifier.StringResult>
        {
            public StringResultComparator(){}
            @Override
            public int compare(Classifier.StringResult p1, Classifier.StringResult p2) {
                return -Float.compare(p1.prob, p2.prob);
            }
        }

        ProbId [] substitutes = new ProbId[ids.length * (ids[0].length - 1)];
        for(int i = 0; i < ids.length; i++){
            for(int j = 1; j < ids[i].length; j++){
                ProbId tmp = new ProbId();
                tmp.i = i;
                tmp.j = j;
                tmp.prob = probs[i][j];
                substitutes[i*(ids[0].length - 1) + j - 1] = tmp;
            }
        }

        //3 sort substitutes
        Arrays.sort(substitutes, new ProbIdComparator());

        //4 get options
        int realNum = 0;
        for(int i = 0; i < substitutes.length; i++){
            if(substitutes[i].prob < probCutoff){
                realNum = i;
                break;
            }
        }

        ProbId [][] prevLevel = new ProbId[realNum][1];
        for(int i = 0; i < realNum; i++){
            prevLevel[i][0] = substitutes[i];
        }
        int [] usedIs = new int [numChanges];
        int [] usedJs = new int [numChanges];
        for(int i = 1; i < numChanges; i++){
            ProbId [][] nextLevel = new ProbId[prevLevel.length * realNum][i+1];
            for(int j = 0; j < prevLevel.length; j++){
                for(int k = 0; k < prevLevel[j].length; k++){
                    for(int m = 0; m < realNum; m++){
                        nextLevel[j*realNum + m][k] = prevLevel[j][k];
                    }
                }
            }
            int newNum = 0;
            for(int j = 0; j < prevLevel.length; j++){
                for(int k = 0; k < realNum; k++){
                    nextLevel[j*realNum+k][i] = substitutes[k];
                    ProbId [] tmp = nextLevel[j * realNum+k];
                    float prob = 1.0f;
                    for(int m = 0; m < i + 1; m++){
                        boolean usedLevel = false;
                        for(int r = 0; r < m; r++){
                            if((usedIs[r] == tmp[m].i) && (usedJs[r] == tmp[m].j)){
                                usedLevel = true;
                                break;
                            }
                        }
                        usedIs[m] = tmp[m].i;
                        usedIs[m] = tmp[m].j;
                        if(usedLevel)
                            continue;
                        prob *= tmp[m].prob;
                    }
                    if(prob > probCutoff)
                        newNum++;
                }
            }
            ProbId [][] newLevel = new ProbId[newNum][];
            int curId = 0;

            for(int j = 0; j < nextLevel.length; j++){
                ProbId [] tmp = nextLevel[j];
                float prob = 1.0f;
                for(int m = 0; m < i + 1; m++){
                    boolean usedLevel = false;
                    for(int r = 0; r < m; r++){
                        if((usedIs[r] == tmp[m].i) && (usedJs[r] == tmp[m].j)){
                            usedLevel = true;
                            break;
                        }
                    }
                    usedIs[m] = tmp[m].i;
                    usedIs[m] = tmp[m].j;
                    if(usedLevel)
                        continue;
                    prob *= tmp[m].prob;
                }
                if(prob > probCutoff){
                    newLevel[curId] = tmp;
                    curId++;
                }
            }

            prevLevel = newLevel;
        }

        //5 compress results
        LinkedList<StringResult> res0 = new LinkedList<StringResult>();
        LinkedList<int []> allDecoded = new LinkedList<int[]>();
        int [] newDecoded = new int[encoded.length];
        float [] newProbs = new float[encoded.length];
        for(int i = 0; i < prevLevel.length; i++){
            for(int j = 0; j < probs.length; j++){
                newDecoded[j] = ids[j][0];
                newProbs[j] = probs[j][0];
            }
            ProbId[] tmp = prevLevel[i];
            for(int j = 0; j < prevLevel[i].length; j++){
                boolean usedLevel = false;
                for(int r = 0; r < j; r++){
                    if((usedIs[r] == tmp[j].i) && (usedJs[r] == tmp[j].j)){
                        usedLevel = true;
                        break;
                    }
                }
                usedIs[j] = tmp[j].i;
                usedIs[j] = tmp[j].j;
                if(usedLevel)
                    continue;

                newDecoded[tmp[j].i] = ids[tmp[j].i][tmp[j].j];
                newProbs[tmp[j].i] = probs[tmp[j].i][tmp[j].j];
            }
            boolean netnew = true;
            for(int j = 0; j < allDecoded.size(); j++){
                boolean same = true;
                int [] tmpv = allDecoded.get(j);
                for(int k = 0; k < newDecoded.length; k++){
                    if(tmpv[k] != newDecoded[k]){
                        same = false;
                        break;
                    }
                }
                if(same){
                    netnew = false;
                    break;
                }
            }
            if(netnew) {
                res0.add(decode(newDecoded, newProbs));
                allDecoded.add(newDecoded.clone());
            }
        }
        for(int j = 0; j < probs.length; j++){
            newDecoded[j] = ids[j][0];
            newProbs[j] = probs[j][0];
        }
        res0.add(decode(newDecoded, newProbs));

        StringResult [] res = new StringResult[res0.size()];
        for(int i = 0; i < res.length; i++){
            res[i] = res0.get(i);
        }

        Arrays.sort(res, new StringResultComparator());

        return res;
    }

    private String decode(int [] encoded){
        int terminal = chars_.length();
        StringBuffer res = new StringBuffer(encoded.length);
        int prev = -1;
        for(int i = 0; i < encoded.length; i++){
            if(encoded[i] == prev)
                continue;
            if(encoded[i] != terminal)
                res.append(chars_.charAt(encoded[i]));
            prev = encoded[i];
        }
        return res.toString();
    }
    private StringResult decode(int [] encoded, float [] probs){
        int terminal = chars_.length();
        double probability = 1.0;
        StringBuffer res = new StringBuffer(encoded.length);
        int prev = -1;
        for(int i = 0; i < encoded.length; i++){
            probability *= probs[i];
            if(encoded[i] == prev)
                continue;
            if(encoded[i] != terminal)
                res.append(chars_.charAt(encoded[i]));
            prev = encoded[i];
        }
        StringResult sr = new StringResult();
        sr.prob = (float)probability;
        sr.string = res.toString();
        return sr;
    }

    private String decodeGreedy(float [][] encoded){
        int [] ids = new int[encoded.length];
        for(int i = 0; i < encoded.length; i++){
            int best = 0;
            for(int j = 1; j < encoded[i].length; j++){
                if(encoded[i][j] > encoded[i][best])
                    best = j;
            }
            ids[i] = best;
        }
        return decode(ids);
    }

    private void applyTransforms(Bitmap src, float cutoff) throws Exception{
        loadBitmapInBuffer(src);
        transformCutoff(cutoff, 1.0f, 0.0f);
        transformTrim(1e-3f);
        transformFitSize(0.0f);
        transformMeanStd(-1.0f);
        transformTranspose();
    }

    private Bitmap arrayToBitmap_(float [][] src, int rows, int cols){
        byte [] bitmapdata = new byte[rows * cols];
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                bitmapdata[i*cols + j] = (byte)(255.0f * src[i][j]);
            }
        }
        Bitmap bmp = Bitmap.createBitmap(cols, rows, Bitmap.Config.ALPHA_8);
        ByteBuffer buffer = ByteBuffer.wrap(bitmapdata);
        bmp.copyPixelsFromBuffer(buffer);
        return bmp;
    }
    private void loadBitmapInBuffer(Bitmap src){
        int rows = src.getHeight();
        int cols = src.getWidth();
        int nbytes = src.getByteCount();

        updateBuffers_(rows, cols);
        int id = nextBuffer_();
        bufferRows_[id] = rows;
        bufferCols_[id] = cols;

        if(src.getConfig() == Bitmap.Config.ALPHA_8){
            byte [] buff = new byte[nbytes];
            src.copyPixelsToBuffer(ByteBuffer.wrap(buff));
            for(int i = 0; i < rows; i++){
                for(int j = 0; j < cols; j++){
                    int tmp = ((int)buff[i*cols+j])&(0xFF);
                    buffers_[id][i][j] = ((float)tmp) / 255.0f;
                }
            }
        }else if(src.getConfig() == Bitmap.Config.ARGB_8888){
            int []tmp = new int[rows * cols];
            src.getPixels(tmp, 0, cols, 0, 0, cols, rows);
            for(int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    int t = tmp[i * cols + j];
                    float f = ((float) (t & 0xFF)) + ((float) ((t >> 8) & 0xFF)) + ((float) ((t >> 16) & 0xFF));
                    buffers_[id][i][j] = f / 255.0f / 3.0f;
                }
            }
        }
    }

    private void transformFitSize(float fillValue){
        Bitmap bmp = arrayToBitmap_(buffers_[activeBuffer_], bufferRows_[activeBuffer_], bufferCols_[activeBuffer_]);
        float fY = ((float)dim_in_rows) / bmp.getHeight();
        float fX = ((float)dim_in_cols) / bmp.getWidth();
        float f = Math.min(fX, fY);
        Bitmap rsz = Bitmap.createScaledBitmap(bmp, (int)(bmp.getWidth()*f), (int)(bmp.getHeight()*f), true);

        loadBitmapInBuffer(rsz);
        if((bufferRows_[activeBuffer_] == dim_in_rows) && (bufferCols_[activeBuffer_] == dim_in_cols))
            return;
        for(int i = bufferRows_[activeBuffer_]; i < dim_in_rows; i++){
            for(int j = 0; j < dim_in_cols; j++){
                buffers_[activeBuffer_][i][j] = fillValue;
            }
        }
        for(int i = 0; i < bufferRows_[activeBuffer_]; i++){
            for(int j = bufferCols_[activeBuffer_]; j < dim_in_cols; j++){
                buffers_[activeBuffer_][i][j] = fillValue;
            }
        }
        bufferRows_[activeBuffer_] = dim_in_rows;
        bufferCols_[activeBuffer_] = dim_in_cols;
    }

    private void transformCutoff(float cutoff, float valueBelow, float valueAbove){
        //1.0*(img < transform.get('cutoff', 0.5)*np.mean(img))
        float sum_x = 0.0f;
        for(int i = 0; i < bufferRows_[activeBuffer_]; i++) {
            for (int j = 0; j < bufferCols_[activeBuffer_]; j++) {
                sum_x += buffers_[activeBuffer_][i][j];
            }
        }
        float mean = sum_x / (bufferRows_[activeBuffer_] * bufferCols_[activeBuffer_]);
        float val = cutoff * mean;
        for(int i = 0; i < bufferRows_[activeBuffer_]; i++){
            for(int j = 0; j < bufferCols_[activeBuffer_]; j++){
                if(buffers_[activeBuffer_][i][j] < val)
                    buffers_[activeBuffer_][i][j] = valueBelow;
                else
                    buffers_[activeBuffer_][i][j] = valueAbove;
            }
        }
    }
    private int findFirstNonzero_(boolean []arr){
        for(int i = 0; i < arr.length; i++){
            if(arr[i])
                return i;
        }
        return arr.length;
    }
    private int findLastNonzero_(boolean []arr){
        for(int i = arr.length-1; i >= 0; i--){
            if(arr[i])
                return i+1;
        }
        return 0;
    }

    private void transformMeanStd(float coef){
        int rows = bufferRows_[activeBuffer_];
        int cols = bufferCols_[activeBuffer_];

        float sum_x = 0.0f, sum_x2 = 0.0f;
        for(int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                sum_x += buffers_[activeBuffer_][i][j];
                sum_x2 += buffers_[activeBuffer_][i][j] * buffers_[activeBuffer_][i][j];
            }
        }
        int num = (rows * cols);
        float mean = sum_x / num;
        float std = (float)Math.sqrt(sum_x2 / num - mean*mean);
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                buffers_[activeBuffer_][i][j] = coef * (buffers_[activeBuffer_][i][j]-mean) / std;
            }
        }
    }
    private void transformMinMax(){
        int rows = bufferRows_[activeBuffer_];
        int cols = bufferCols_[activeBuffer_];

        float min = buffers_[activeBuffer_][0][0], max = buffers_[activeBuffer_][0][0];
        for(int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                min = Math.min(min, buffers_[activeBuffer_][i][j]);
                max = Math.max(max, buffers_[activeBuffer_][i][j]);
            }
        }
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                buffers_[activeBuffer_][i][j] = (buffers_[activeBuffer_][i][j] - min) / (max - min);
            }
        }
    }
    private void transformTranspose(){
        int rows = bufferRows_[activeBuffer_];
        int cols = bufferCols_[activeBuffer_];
        int prev_id = activeBuffer_;
        int id = nextBuffer_();
        bufferCols_[id] = rows;
        bufferRows_[id] = cols;
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                buffers_[id][j][i] = buffers_[prev_id][i][j];
            }
        }
    }
    private void transformTrim(float cutoff) throws Exception{
        int rows = bufferRows_[activeBuffer_];
        int cols = bufferCols_[activeBuffer_];
        boolean [] col_any = new boolean[cols];
        boolean [] row_any = new boolean[rows];
        for(int i = 0; i < cols; i++){
            col_any[i] = false;
        }
        for(int i = 0; i < rows; i++){
            row_any[i] = false;
        }
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                if(buffers_[activeBuffer_][i][j] > cutoff){
                    col_any[j] = true;
                    row_any[i] = true;
                }
            }
        }
        int prev_id = activeBuffer_;
        int id = nextBuffer_();
        int col_id1 = findFirstNonzero_(col_any);
        int col_id2 = findLastNonzero_(col_any);
        int row_id1 = findFirstNonzero_(row_any);
        int row_id2 = findLastNonzero_(row_any);

        if((col_id1 >= col_id2)||(row_id1 >= row_id2)){
            throw new Exception("trimArray: image is empty!");
        }
        bufferRows_[id] = row_id2 - row_id1;
        bufferCols_[id] = col_id2 - col_id1;
        for(int i = 0; i < row_id2-row_id1; i++) {
            for (int j = 0; j < col_id2-col_id1; j++) {
                buffers_[id][i][j] = buffers_[prev_id][row_id1+i][col_id1+j];
            }
        }
    }
}
