namespace PageLabeler
{
    partial class PageLabeler
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.splitThumb = new System.Windows.Forms.SplitContainer();
            this.splitView = new System.Windows.Forms.SplitContainer();
            this.splitMainView = new System.Windows.Forms.SplitContainer();
            this.pbMain = new System.Windows.Forms.PictureBox();
            this.btnDetect = new System.Windows.Forms.Button();
            this.detectorStatus = new System.Windows.Forms.Label();
            this.btnDetectorSettings = new System.Windows.Forms.Button();
            this.splitThumbView = new System.Windows.Forms.SplitContainer();
            this.btnOuput = new System.Windows.Forms.Button();
            this.btnInput = new System.Windows.Forms.Button();
            this.thumbFlowPanel = new System.Windows.Forms.FlowLayoutPanel();
            this.btnLoadDataset = new System.Windows.Forms.Button();
            this.btnSave = new System.Windows.Forms.Button();
            this.pbWord = new System.Windows.Forms.PictureBox();
            this.txtWord = new System.Windows.Forms.TextBox();
            this.rbWordCorrect = new System.Windows.Forms.RadioButton();
            this.rbWordIncorrect = new System.Windows.Forms.RadioButton();
            this.txtWordComment = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.splitThumb)).BeginInit();
            this.splitThumb.Panel1.SuspendLayout();
            this.splitThumb.Panel2.SuspendLayout();
            this.splitThumb.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitView)).BeginInit();
            this.splitView.Panel1.SuspendLayout();
            this.splitView.Panel2.SuspendLayout();
            this.splitView.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitMainView)).BeginInit();
            this.splitMainView.Panel1.SuspendLayout();
            this.splitMainView.Panel2.SuspendLayout();
            this.splitMainView.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbMain)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.splitThumbView)).BeginInit();
            this.splitThumbView.Panel1.SuspendLayout();
            this.splitThumbView.Panel2.SuspendLayout();
            this.splitThumbView.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbWord)).BeginInit();
            this.SuspendLayout();
            // 
            // splitThumb
            // 
            this.splitThumb.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.splitThumb.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitThumb.Location = new System.Drawing.Point(0, 0);
            this.splitThumb.Name = "splitThumb";
            // 
            // splitThumb.Panel1
            // 
            this.splitThumb.Panel1.Controls.Add(this.splitThumbView);
            // 
            // splitThumb.Panel2
            // 
            this.splitThumb.Panel2.Controls.Add(this.splitView);
            this.splitThumb.Size = new System.Drawing.Size(1221, 566);
            this.splitThumb.SplitterDistance = 407;
            this.splitThumb.TabIndex = 0;
            // 
            // splitView
            // 
            this.splitView.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.splitView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitView.Location = new System.Drawing.Point(0, 0);
            this.splitView.Name = "splitView";
            // 
            // splitView.Panel1
            // 
            this.splitView.Panel1.Controls.Add(this.splitMainView);
            // 
            // splitView.Panel2
            // 
            this.splitView.Panel2.Controls.Add(this.label2);
            this.splitView.Panel2.Controls.Add(this.label1);
            this.splitView.Panel2.Controls.Add(this.txtWordComment);
            this.splitView.Panel2.Controls.Add(this.rbWordIncorrect);
            this.splitView.Panel2.Controls.Add(this.rbWordCorrect);
            this.splitView.Panel2.Controls.Add(this.txtWord);
            this.splitView.Panel2.Controls.Add(this.pbWord);
            this.splitView.Size = new System.Drawing.Size(810, 566);
            this.splitView.SplitterDistance = 350;
            this.splitView.TabIndex = 0;
            // 
            // splitMainView
            // 
            this.splitMainView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitMainView.Location = new System.Drawing.Point(0, 0);
            this.splitMainView.Name = "splitMainView";
            this.splitMainView.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitMainView.Panel1
            // 
            this.splitMainView.Panel1.Controls.Add(this.btnDetectorSettings);
            this.splitMainView.Panel1.Controls.Add(this.detectorStatus);
            this.splitMainView.Panel1.Controls.Add(this.btnDetect);
            // 
            // splitMainView.Panel2
            // 
            this.splitMainView.Panel2.Controls.Add(this.pbMain);
            this.splitMainView.Size = new System.Drawing.Size(348, 564);
            this.splitMainView.SplitterDistance = 80;
            this.splitMainView.TabIndex = 0;
            // 
            // pbMain
            // 
            this.pbMain.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pbMain.Location = new System.Drawing.Point(0, 0);
            this.pbMain.Name = "pbMain";
            this.pbMain.Size = new System.Drawing.Size(348, 480);
            this.pbMain.TabIndex = 10;
            this.pbMain.TabStop = false;
            // 
            // btnDetect
            // 
            this.btnDetect.Location = new System.Drawing.Point(3, 12);
            this.btnDetect.Name = "btnDetect";
            this.btnDetect.Size = new System.Drawing.Size(125, 38);
            this.btnDetect.TabIndex = 14;
            this.btnDetect.Text = "Detect words";
            this.btnDetect.UseVisualStyleBackColor = true;
            this.btnDetect.Click += new System.EventHandler(this.BtnDetect_Click);
            // 
            // detectorStatus
            // 
            this.detectorStatus.AutoSize = true;
            this.detectorStatus.Location = new System.Drawing.Point(3, 53);
            this.detectorStatus.Name = "detectorStatus";
            this.detectorStatus.Size = new System.Drawing.Size(77, 20);
            this.detectorStatus.TabIndex = 15;
            this.detectorStatus.Text = "No status";
            // 
            // btnDetectorSettings
            // 
            this.btnDetectorSettings.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnDetectorSettings.Location = new System.Drawing.Point(134, 12);
            this.btnDetectorSettings.Name = "btnDetectorSettings";
            this.btnDetectorSettings.Size = new System.Drawing.Size(151, 38);
            this.btnDetectorSettings.TabIndex = 16;
            this.btnDetectorSettings.Text = "Detector settings";
            this.btnDetectorSettings.UseVisualStyleBackColor = true;
            // 
            // splitThumbView
            // 
            this.splitThumbView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitThumbView.Location = new System.Drawing.Point(0, 0);
            this.splitThumbView.Name = "splitThumbView";
            this.splitThumbView.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitThumbView.Panel1
            // 
            this.splitThumbView.Panel1.Controls.Add(this.btnSave);
            this.splitThumbView.Panel1.Controls.Add(this.btnLoadDataset);
            this.splitThumbView.Panel1.Controls.Add(this.btnInput);
            this.splitThumbView.Panel1.Controls.Add(this.btnOuput);
            // 
            // splitThumbView.Panel2
            // 
            this.splitThumbView.Panel2.Controls.Add(this.thumbFlowPanel);
            this.splitThumbView.Size = new System.Drawing.Size(405, 564);
            this.splitThumbView.SplitterDistance = 150;
            this.splitThumbView.TabIndex = 0;
            // 
            // btnOuput
            // 
            this.btnOuput.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOuput.Location = new System.Drawing.Point(142, 12);
            this.btnOuput.Name = "btnOuput";
            this.btnOuput.Size = new System.Drawing.Size(125, 38);
            this.btnOuput.TabIndex = 8;
            this.btnOuput.Text = "Set output";
            this.btnOuput.UseVisualStyleBackColor = true;
            this.btnOuput.Click += new System.EventHandler(this.BtnOuput_Click);
            // 
            // btnInput
            // 
            this.btnInput.Location = new System.Drawing.Point(11, 12);
            this.btnInput.Name = "btnInput";
            this.btnInput.Size = new System.Drawing.Size(125, 38);
            this.btnInput.TabIndex = 9;
            this.btnInput.Text = "Add input";
            this.btnInput.UseVisualStyleBackColor = true;
            this.btnInput.Click += new System.EventHandler(this.BtnInput_Click);
            // 
            // thumbFlowPanel
            // 
            this.thumbFlowPanel.AutoScroll = true;
            this.thumbFlowPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.thumbFlowPanel.Location = new System.Drawing.Point(0, 0);
            this.thumbFlowPanel.Name = "thumbFlowPanel";
            this.thumbFlowPanel.Size = new System.Drawing.Size(405, 410);
            this.thumbFlowPanel.TabIndex = 2;
            // 
            // btnLoadDataset
            // 
            this.btnLoadDataset.Location = new System.Drawing.Point(11, 56);
            this.btnLoadDataset.Name = "btnLoadDataset";
            this.btnLoadDataset.Size = new System.Drawing.Size(125, 38);
            this.btnLoadDataset.TabIndex = 10;
            this.btnLoadDataset.Text = "Load dataset";
            this.btnLoadDataset.UseVisualStyleBackColor = true;
            this.btnLoadDataset.Click += new System.EventHandler(this.BtnLoadDataset_Click);
            // 
            // btnSave
            // 
            this.btnSave.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnSave.Location = new System.Drawing.Point(142, 56);
            this.btnSave.Name = "btnSave";
            this.btnSave.Size = new System.Drawing.Size(125, 38);
            this.btnSave.TabIndex = 11;
            this.btnSave.Text = "Save";
            this.btnSave.UseVisualStyleBackColor = true;
            this.btnSave.Click += new System.EventHandler(this.BtnSave_Click);
            // 
            // pbWord
            // 
            this.pbWord.Dock = System.Windows.Forms.DockStyle.Top;
            this.pbWord.Location = new System.Drawing.Point(0, 0);
            this.pbWord.Name = "pbWord";
            this.pbWord.Size = new System.Drawing.Size(454, 141);
            this.pbWord.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pbWord.TabIndex = 0;
            this.pbWord.TabStop = false;
            // 
            // txtWord
            // 
            this.txtWord.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.txtWord.Location = new System.Drawing.Point(3, 179);
            this.txtWord.Name = "txtWord";
            this.txtWord.Size = new System.Drawing.Size(444, 35);
            this.txtWord.TabIndex = 17;
            this.txtWord.TextChanged += new System.EventHandler(this.TxtWord_TextChanged);
            // 
            // rbWordCorrect
            // 
            this.rbWordCorrect.AutoSize = true;
            this.rbWordCorrect.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.rbWordCorrect.Location = new System.Drawing.Point(132, 142);
            this.rbWordCorrect.Name = "rbWordCorrect";
            this.rbWordCorrect.Size = new System.Drawing.Size(101, 29);
            this.rbWordCorrect.TabIndex = 18;
            this.rbWordCorrect.TabStop = true;
            this.rbWordCorrect.Text = "Correct";
            this.rbWordCorrect.UseVisualStyleBackColor = true;
            this.rbWordCorrect.CheckedChanged += new System.EventHandler(this.RbWordCorrect_CheckedChanged);
            // 
            // rbWordIncorrect
            // 
            this.rbWordIncorrect.AutoSize = true;
            this.rbWordIncorrect.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.rbWordIncorrect.Location = new System.Drawing.Point(239, 142);
            this.rbWordIncorrect.Name = "rbWordIncorrect";
            this.rbWordIncorrect.Size = new System.Drawing.Size(112, 29);
            this.rbWordIncorrect.TabIndex = 19;
            this.rbWordIncorrect.TabStop = true;
            this.rbWordIncorrect.Text = "Incorrect";
            this.rbWordIncorrect.UseVisualStyleBackColor = true;
            this.rbWordIncorrect.CheckedChanged += new System.EventHandler(this.RbWordIncorrect_CheckedChanged);
            // 
            // txtWordComment
            // 
            this.txtWordComment.Font = new System.Drawing.Font("Microsoft Sans Serif", 8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.txtWordComment.Location = new System.Drawing.Point(3, 263);
            this.txtWordComment.Multiline = true;
            this.txtWordComment.Name = "txtWordComment";
            this.txtWordComment.Size = new System.Drawing.Size(446, 64);
            this.txtWordComment.TabIndex = 20;
            this.txtWordComment.TextChanged += new System.EventHandler(this.TxtWordComment_TextChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.label1.Location = new System.Drawing.Point(3, 144);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(123, 25);
            this.label1.TabIndex = 17;
            this.label1.Text = "Word status:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.label2.Location = new System.Drawing.Point(3, 235);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(151, 25);
            this.label2.TabIndex = 21;
            this.label2.Text = "Word comment:";
            // 
            // PageLabeler
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1221, 566);
            this.Controls.Add(this.splitThumb);
            this.KeyPreview = true;
            this.Name = "PageLabeler";
            this.Text = "Page labeler";
            this.Load += new System.EventHandler(this.PageLabeler_Load);
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.PageLabeler_KeyDown);
            this.splitThumb.Panel1.ResumeLayout(false);
            this.splitThumb.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitThumb)).EndInit();
            this.splitThumb.ResumeLayout(false);
            this.splitView.Panel1.ResumeLayout(false);
            this.splitView.Panel2.ResumeLayout(false);
            this.splitView.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitView)).EndInit();
            this.splitView.ResumeLayout(false);
            this.splitMainView.Panel1.ResumeLayout(false);
            this.splitMainView.Panel1.PerformLayout();
            this.splitMainView.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitMainView)).EndInit();
            this.splitMainView.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pbMain)).EndInit();
            this.splitThumbView.Panel1.ResumeLayout(false);
            this.splitThumbView.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitThumbView)).EndInit();
            this.splitThumbView.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pbWord)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitThumb;
        private System.Windows.Forms.SplitContainer splitView;
        private System.Windows.Forms.SplitContainer splitMainView;
        private System.Windows.Forms.Label detectorStatus;
        private System.Windows.Forms.Button btnDetect;
        private System.Windows.Forms.PictureBox pbMain;
        private System.Windows.Forms.Button btnDetectorSettings;
        private System.Windows.Forms.SplitContainer splitThumbView;
        private System.Windows.Forms.Button btnInput;
        private System.Windows.Forms.Button btnOuput;
        private System.Windows.Forms.FlowLayoutPanel thumbFlowPanel;
        private System.Windows.Forms.Button btnLoadDataset;
        private System.Windows.Forms.Button btnSave;
        private System.Windows.Forms.PictureBox pbWord;
        private System.Windows.Forms.TextBox txtWord;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtWordComment;
        private System.Windows.Forms.RadioButton rbWordIncorrect;
        private System.Windows.Forms.RadioButton rbWordCorrect;
    }
}

