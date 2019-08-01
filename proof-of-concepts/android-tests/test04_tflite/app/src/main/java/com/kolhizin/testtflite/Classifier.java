package com.kolhizin.testtflite;

import android.app.Activity;
import android.content.res.AssetFileDescriptor;
import android.graphics.Bitmap;
import android.os.SystemClock;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;

import org.tensorflow.lite.Interpreter;
import org.tensorflow.lite.Tensor;

public class Classifier {
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
            growBuffers_(dim*2, dim*2);
    }

    private void growBuffers_(int rows, int cols){
        int nr = Math.max(rows, buffers_[0].length);
        int nc = Math.max(cols, buffers_[0][0].length);
        int nb = buffers_.length;
        float [][][] res = new float[nb][nr][nc];
        for(int i = 0; i < nb; i++){
            for(int j = 0; j < nr; j++){
                for(int k = 0; k < nc; k++){
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
        if (input.length != dim_in_rows)
            throw new RuntimeException("Input row number does not match expected by model!");
        if (input[0].length != dim_in_cols)
            throw new RuntimeException("Input col number does not match expected by model!");
        inputData_.rewind();
        for (int i = 0; i < dim_in_rows; ++i) {
            for (int j = 0; j < dim_in_cols; ++j) {
                inputData_.putFloat(input[i][j]);
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
    private void transformTranspose(){
        int rows = bufferRows_[activeBuffer_];
        int cols = bufferCols_[activeBuffer_];
        int id = nextBuffer_();
        bufferCols_[id] = rows;
        bufferRows_[id] = cols;
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                activeBuffer_[id][j][i] = activeBuffer_[activeBuffer_][i][j];
            }
        }
    }
    /*
    private float [][] transformTrim(float [][]arr, float cutoff) throws Exception{
        int rows = arr.length;
        int cols = arr[0].length;
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
                if(arr[i][j] > cutoff){
                    col_any[j] = true;
                    row_any[i] = true;
                }
            }
        }

        int col_id1 = findFirstNonzero(col_any);
        int col_id2 = findLastNonzero(col_any);
        int row_id1 = findFirstNonzero(row_any);
        int row_id2 = findLastNonzero(row_any);

        if((col_id1 >= col_id2)||(row_id1 >= row_id2)){
            throw new Exception("trimArray: image is empty!");
        }
        float [][] res = new float[row_id2-row_id1][col_id2-col_id1];
        for(int i = 0; i < row_id2-row_id1; i++) {
            for (int j = 0; j < col_id2-col_id1; j++) {
                res[i][j] = arr[row_id1+i][col_id1+j];
            }
        }
        return res;
    }
     */
}
