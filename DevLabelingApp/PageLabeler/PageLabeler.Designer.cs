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
            this.thumbFlowPanel = new System.Windows.Forms.FlowLayoutPanel();
            this.btnOuput = new System.Windows.Forms.Button();
            this.btnInput = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.splitThumb)).BeginInit();
            this.splitThumb.Panel1.SuspendLayout();
            this.splitThumb.Panel2.SuspendLayout();
            this.splitThumb.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitView)).BeginInit();
            this.splitView.SuspendLayout();
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
            this.splitThumb.Panel1.Controls.Add(this.btnInput);
            this.splitThumb.Panel1.Controls.Add(this.btnOuput);
            this.splitThumb.Panel1.Controls.Add(this.thumbFlowPanel);
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
            this.splitView.Size = new System.Drawing.Size(810, 566);
            this.splitView.SplitterDistance = 438;
            this.splitView.TabIndex = 0;
            // 
            // thumbFlowPanel
            // 
            this.thumbFlowPanel.AutoScroll = true;
            this.thumbFlowPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.thumbFlowPanel.Location = new System.Drawing.Point(0, 171);
            this.thumbFlowPanel.Name = "thumbFlowPanel";
            this.thumbFlowPanel.Size = new System.Drawing.Size(405, 393);
            this.thumbFlowPanel.TabIndex = 0;
            // 
            // btnOuput
            // 
            this.btnOuput.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOuput.Location = new System.Drawing.Point(279, 12);
            this.btnOuput.Name = "btnOuput";
            this.btnOuput.Size = new System.Drawing.Size(125, 38);
            this.btnOuput.TabIndex = 4;
            this.btnOuput.Text = "Set output";
            this.btnOuput.UseVisualStyleBackColor = true;
            // 
            // btnInput
            // 
            this.btnInput.Location = new System.Drawing.Point(12, 12);
            this.btnInput.Name = "btnInput";
            this.btnInput.Size = new System.Drawing.Size(125, 38);
            this.btnInput.TabIndex = 5;
            this.btnInput.Text = "Set input";
            this.btnInput.UseVisualStyleBackColor = true;
            this.btnInput.Click += new System.EventHandler(this.BtnInput_Click);
            // 
            // PageLabeler
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1221, 566);
            this.Controls.Add(this.splitThumb);
            this.Name = "PageLabeler";
            this.Text = "Page labeler";
            this.Load += new System.EventHandler(this.PageLabeler_Load);
            this.splitThumb.Panel1.ResumeLayout(false);
            this.splitThumb.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitThumb)).EndInit();
            this.splitThumb.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitView)).EndInit();
            this.splitView.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitThumb;
        private System.Windows.Forms.FlowLayoutPanel thumbFlowPanel;
        private System.Windows.Forms.SplitContainer splitView;
        private System.Windows.Forms.Button btnInput;
        private System.Windows.Forms.Button btnOuput;
    }
}

