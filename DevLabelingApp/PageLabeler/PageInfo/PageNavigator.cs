using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;

namespace PageLabeler.PageInfo
{
    class PageNavigator
    {
        private string path_;
        private PageInfo curPage_;
        private LinkedListNode<LineInfo> lineIter_;
        private LinkedListNode<WordInfo> wordIter_;
        private LinkedListNode<PageError> errorIter_;
        private Bitmap refImage_;

        public PageNavigator()
        {
            curPage_ = null;
        }
        public void SetPath(string path)
        {
            path_ = path;
        }

        public void SetPage(PageInfo page, Image refImage)
        {
            curPage_ = page;
            refImage_ = (Bitmap)refImage;
            lineIter_ = curPage_.lines.First;
            wordIter_ = null;
            errorIter_ = curPage_.errors.First;
            while (lineIter_ != null)
            {
                wordIter_ = lineIter_.Value.words.First;
                if (wordIter_ != null)
                    return;
                lineIter_ = lineIter_.Next;
            }
        }

        public Image GetDetectorAlignedImage()
        {
            if(curPage_ == null || path_ == "")
                return null;
            return curPage_.GetDetectorAlignedImage(path_);
        }
        public Image GetDetectorFilteredImage()
        {
            if (curPage_ == null || path_ == "")
                return null;
            return curPage_.GetDetectorFilteredImage(path_);
        }
        public Image GetDetectorMainImage()
        {
            if (curPage_ == null || path_ == "")
                return null;
            return curPage_.GetDetectorMainImage(path_);
        }
        public Image GetDetectorTextImage()
        {
            if (curPage_ == null || path_ == "")
                return null;
            return curPage_.GetDetectorTextImage(path_);
        }

        public bool NextError()
        {
            if (errorIter_ == null)
                return false;
            var err = errorIter_.Next;
            if (err == null)
                return false;
            errorIter_ = err;
            return true;
        }
        public bool PrevError()
        {
            if (errorIter_ == null)
                return false;
            var err = errorIter_.Previous;
            if (err == null)
                return false;
            errorIter_ = err;
            return true;
        }

        public bool NextWord(HashSet<WordInfo.WordStatus> skip)
        {
            if (wordIter_ == null || lineIter_ == null)
                return false;

            var word = wordIter_.Next;
            var line = lineIter_;
            while(word == null || skip.Contains(word.Value.status))
            {
                if (word == null)
                {
                    line = line.Next;
                    if (line == null)
                        return false;
                    word = line.Value.words.First;
                }
                else
                    word = word.Next;
            }
            wordIter_ = word;
            lineIter_ = line;
            return true;
        }
        public bool PrevWord(HashSet<WordInfo.WordStatus> skip)
        {
            if (wordIter_ == null || lineIter_ == null)
                return false;

            var word = wordIter_.Previous;
            var line = lineIter_;
            while (word == null || skip.Contains(word.Value.status))
            {
                if (word == null)
                {
                    line = line.Previous;
                    if (line == null)
                        return false;
                    word = line.Value.words.Last;
                }
                else
                    word = word.Previous;
            }
            wordIter_ = word;
            lineIter_ = line;
            return true;
        }
        public bool NextLine(HashSet<WordInfo.WordStatus> skip)
        {
            if (wordIter_ == null || lineIter_ == null)
                return false;

            var line = lineIter_.Next;
            var word = line.Value.words.First;
            
            while (word == null || skip.Contains(word.Value.status))
            {
                line = line.Next;
                if (line == null)
                    return false;
                word = line.Value.words.First;
            }
            wordIter_ = word;
            lineIter_ = line;
            return true;
        }
        public bool PrevLine(HashSet<WordInfo.WordStatus> skip)
        {
            if (wordIter_ == null || lineIter_ == null)
                return false;

            var line = lineIter_.Previous;
            var word = line.Value.words.Last;

            while (word == null || skip.Contains(word.Value.status))
            {
                line = line.Next;
                if (line == null)
                    return false;
                word = line.Value.words.First;
            }
            wordIter_ = word;
            lineIter_ = line;
            return true;
        }
        public Image GetWordImage(string path)
        {
            if (wordIter_ == null)
                return null;
            string fname = System.IO.Path.Combine(path, curPage_.name, wordIter_.Value.filename);
            return Image.FromFile(fname);
        }
        public WordInfo.WordStatus GetWordStatus()
        {
            if (wordIter_ == null)
                return WordInfo.WordStatus.UNKNOWN;
            return wordIter_.Value.status;
        }
        public void SetWordStatus(WordInfo.WordStatus status)
        {
            if (wordIter_ == null)
                return;
            wordIter_.Value.status = status;
        }
        public string GetWordText()
        {
            if (wordIter_ == null)
                return "";
            return wordIter_.Value.text;
        }
        public void SetWordText(string str)
        {
            if (wordIter_ == null)
                return;
            wordIter_.Value.text = str;
        }
        public string GetWordComment()
        {
            if (wordIter_ == null)
                return "";
            return wordIter_.Value.comment;
        }
        public void SetWordComment(string str)
        {
            if (wordIter_ == null)
                return;
            wordIter_.Value.comment = str;
        }
        public void SetLineStatus(LineInfo.LineStatus status)
        {
            if (lineIter_ == null)
                return;
            lineIter_.Value.status = status;
            if(status == LineInfo.LineStatus.INCORRECT)
            {
                var wi = lineIter_.Value.words.First;
                while (wi != null)
                {
                    wi.Value.status = WordInfo.WordStatus.INCORRECT;
                    wi = wi.Next;
                }
            }
        }
        public LineInfo.LineStatus GetLineStatus()
        {
            if (lineIter_ == null)
                return LineInfo.LineStatus.UNKNOWN;
            return lineIter_.Value.status;
        }

        public void SetLineComment(string str)
        {
            if (lineIter_ == null)
                return;
            lineIter_.Value.comment = str;
        }

        public string GetLineComment()
        {
            if (lineIter_ == null)
                return "";
            return lineIter_.Value.comment;
        }

        public WordInfo.WordStatus GetWordStatus(int lineId, int wordId)
        {
            if (curPage_ == null)
                return WordInfo.WordStatus.UNKNOWN;
            return curPage_.lines.ElementAt(lineId).words.ElementAt(wordId).status;
        }
        public RectangleF GetWordBBox(int lineId, int wordId, float yScale)
        {
            return curPage_.lines.ElementAt(lineId).words.ElementAt(wordId).GetBBox(yScale);
        }
        public bool IsSelected(int lineId, int wordId)
        {
            if (wordIter_ == null)
                return false;
            return (wordIter_.Value.lineId == lineId && wordIter_.Value.wordId == wordId);
        }
        public int NumLines()
        {
            if (curPage_ == null)
                return 0;
            return curPage_.lines.Count;
        }
        public int NumWords(int lineId)
        {
            if (curPage_ == null)
                return 0;
            return curPage_.lines.ElementAt(lineId).words.Count;
        }
        public int NumErrors()
        {
            if (curPage_ == null)
                return 0;
            return curPage_.errors.Count;
        }
        public void SelectLastError()
        {
            if (curPage_ == null)
                return;
            errorIter_ = curPage_.errors.Last;
        }

        public void AddError(float x, float y, PageError.ErrorType errorType)
        {
            if (curPage_ == null)
                return;
            PageError pe = new PageError();
            pe.x = x;
            pe.y = y;
            pe.type = errorType;
            curPage_.errors.AddLast(pe);
        }
        public void RemoveError()
        {
            if (curPage_ == null)
                return;
            var node = errorIter_.Next;
            if (node == null)
                node = errorIter_.Previous;
            curPage_.errors.Remove(errorIter_);
            errorIter_ = node;
            if (errorIter_ == null)
                errorIter_ = curPage_.errors.Last;
        }
        
        public PageError GetError()
        {
            if (curPage_ == null|| errorIter_ == null)
                return null;
            return errorIter_.Value;
        }

        public PageError GetError(int id)
        {
            if (curPage_ == null)
                return null;
            return curPage_.errors.ElementAt(id);
        }
    }
}
