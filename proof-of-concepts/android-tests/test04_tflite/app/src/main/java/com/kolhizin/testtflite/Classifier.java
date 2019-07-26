package com.kolhizin.testtflite;

import android.app.Activity;
import android.content.res.AssetFileDescriptor;
import android.os.SystemClock;

import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import org.tensorflow.lite.Interpreter;

public class Classifier {
    private Interpreter tflite_;
    private Activity activity_;
    private ByteBuffer inputData_ = null;
    private float[][] outputData_ = null;
    public final int batch_size = 1;
    public final int input_rows = 128;
    public final int input_cols = 32;
    public final int output_rows = 32;
    public final int output_cols = 102;


    public Classifier(Activity activity, String modelPath) throws IOException {
        activity_ = activity;

        tflite_ = new Interpreter(loadModelFile(modelPath));
        inputData_ = ByteBuffer.allocateDirect(batch_size * input_rows * input_cols * 4);
        inputData_.order(ByteOrder.nativeOrder());
        outputData_ = new float[output_rows][output_cols];
    }

    private MappedByteBuffer loadModelFile(String modelPath) throws IOException {
        AssetFileDescriptor fileDescriptor = activity_.getAssets().openFd(modelPath);
        FileInputStream inputStream = new FileInputStream(fileDescriptor.getFileDescriptor());
        FileChannel fileChannel = inputStream.getChannel();
        long startOffset = fileDescriptor.getStartOffset();
        long declaredLength = fileDescriptor.getDeclaredLength();
        return fileChannel.map(FileChannel.MapMode.READ_ONLY, startOffset, declaredLength);
    }

    private void setInput(float [][] input) throws RuntimeException{
        if (inputData_ == null)
            throw new RuntimeException("Internal input buffer is not initialized!");
        if (input.length != input_rows)
            throw new RuntimeException("Input row number does not match expected by model!");
        if (input[0].length != input_cols)
            throw new RuntimeException("Input col number does not match expected by model!");
        inputData_.rewind();
        for (int i = 0; i < input_rows; ++i) {
            for (int j = 0; j < input_cols; ++j) {
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
        return outputData_;
    }

    /*
    // Runs inference and returns the classification results.
    public List<Recognition> recognizeImage(final Bitmap bitmap) {
        // Run the inference call.
        // Find the best classifications.
        PriorityQueue<Recognition> pq =
                new PriorityQueue<Recognition>(
                        3,
                        new Comparator<Recognition>() {
                            @Override
                            public int compare(Recognition lhs, Recognition rhs) {
                                // Intentionally reversed to put high confidence at the head of the queue.
                                return Float.compare(rhs.getConfidence(), lhs.getConfidence());
                            }
                        });
        for (int i = 0; i < labels.size(); ++i) {
            pq.add(
                    new Recognition(
                            "" + i,
                            labels.size() > i ? labels.get(i) : "unknown",
                            getNormalizedProbability(i),
                            null));
        }
        final ArrayList<Recognition> recognitions = new ArrayList<Recognition>();
        int recognitionsSize = Math.min(pq.size(), MAX_RESULTS);
        for (int i = 0; i < recognitionsSize; ++i) {
            recognitions.add(pq.poll());
        }
        Trace.endSection();
        return recognitions;
    }
     */
}
