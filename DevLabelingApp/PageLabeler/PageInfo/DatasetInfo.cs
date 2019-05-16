using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PageLabeler.PageInfo
{
    class DatasetInfo
    {
        public Dictionary<string, PageInfo> pages;
        string fname;

        public DatasetInfo(string filename)
        {
            fname = filename;
            Load();
        }
        public DatasetInfo()
        {
            fname = "";
            pages = new Dictionary<string, PageInfo>();
        }

        public void Load()
        {
            if (System.IO.File.Exists(fname))
            {
                string res = System.IO.File.ReadAllText(fname);
                pages = Newtonsoft.Json.JsonConvert.DeserializeObject<Dictionary<string, PageInfo>>(res);
            }else
            {
                pages = new Dictionary<string, PageInfo>();
            }
        }
        public void Save()
        {
            string res = Newtonsoft.Json.JsonConvert.SerializeObject(pages);
            System.IO.File.WriteAllText(fname, res);
        }

        public void UpdateOutput(string filename)
        {
            fname = filename;
            Save();
        }

        public void AddPage(string filename)
        {
            if(pages.ContainsKey(filename))
                throw new Exception("Observation with this filename already in data-set!");
            pages[filename] = new PageInfo();
        }

        public void ResetPage(string filename, BuJoDetector.ManagedDetector detector)
        {
            if (!pages.ContainsKey(filename))
                throw new Exception("Observation with this filename is not in data-set!");
            pages[filename] = new PageInfo(detector);
        }

        public PageInfo GetPage(string filename)
        {
            if (!pages.ContainsKey(filename))
                throw new Exception("Observation with this filename is not in data-set!");
            return pages[filename];
        }
    }
}
