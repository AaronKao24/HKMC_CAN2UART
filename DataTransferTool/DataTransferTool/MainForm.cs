using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;
using System.IO;
using System.Drawing.Imaging;
using System.Diagnostics;

namespace DataTransferTool
{
    public partial class MainForm : Form
    {
        ToolTip file_tool_tip = new ToolTip();
        ToolTip rom_tool_tip = new ToolTip();
        public MainForm()
        {
            InitializeComponent();
            
            string[] myPort;
            string addName;
            myPort = SerialPort.GetPortNames();
            foreach (string pname in myPort)
            {
                if (!Char.IsNumber(pname[pname.Length - 1]))
                {
                    addName = pname.Substring(0, pname.Length - 1);
                }
                else
                {
                    addName = pname;
                }
                cb_com_port.Items.Add(addName);
            }



            file_tool_tip.ToolTipIcon = ToolTipIcon.None;
            file_tool_tip.ForeColor = Color.Blue;
            file_tool_tip.BackColor = Color.Gray;
        //    file_tool_tip.AutoPopDelay = 5000; //AutoPopDelay：當游標停滯在控制項，顯示提示視窗的時間。(以毫秒為單位)
            file_tool_tip.ToolTipTitle = "";

            rom_tool_tip.ToolTipIcon = ToolTipIcon.None;
            rom_tool_tip.ForeColor = Color.Blue;
            rom_tool_tip.BackColor = Color.Gray;
        //    rom_tool_tip.AutoPopDelay = 5000; //AutoPopDelay：當游標停滯在控制項，顯示提示視窗的時間。(以毫秒為單位)
            rom_tool_tip.ToolTipTitle = "";

            tell_thread_break = false;
            Thread thread_receive = new Thread(Receive_Thread);//指派工作給執行緒
            thread_receive.Start();//開始執行緒工作 
            read_size = 2;
            trans_function = 0;
        }

        int trans_function ;

        int read_size;
        byte[] in_datas = new byte[256];
        byte[] ok_din = new byte[2] { (byte)'O', (byte)'K' };
        /**
         * @fn Receive_Thread()
         * @brief 
         */
        void Receive_Thread()
        {
            int geted_size = 0 ;
            int tout_cnt = 0;
            while (true)
            {
                if (tell_thread_break) {
                    break;
                }
                if (data_port.IsOpen == false)
                {
                    continue;
                }
                try
                {
                    geted_size += data_port.Read(in_datas, geted_size, read_size - geted_size);
                }
                catch (IOException ioerr)
                {
                    break;
                }
                catch (TimeoutException tout)
                {
                    if (trans_function >= 2 )
                    {
                        tout_cnt++;
                        if (tout_cnt > 400)
                        {
                            trans_function = 0;
                            ShowButton(true);
                            ShowData("\r\ntimeout " + send_offset + "\r\n");
                        }
                    }
                }
                switch (trans_function)
                {
                    case 0 : // 
                        if (geted_size == read_size)
                        {
                            if (in_datas.SequenceEqual(ok_din)) 
                            {
                                System.Console.WriteLine("OK !");
                            }
                            geted_size = 0;
                        }
                        tout_cnt = 0;
                        break;
                    case 1:
                        ShowData("ROM B " + total_offset.ToString("X4") + "h " + sent_size + "\r\n");
                        data_port.WriteLine("ROM B " + total_offset.ToString("X4") + "h " + sent_size);
                        trans_function = 2;
                        break;
                    case 2:
                        if (geted_size == read_size)
                        {
                            sent_first_big_data_command();
                            trans_function = 3;
                            geted_size = 0;
                            tout_cnt = 0;
                        }
                        break;
                    case 3:
                        if (geted_size == read_size)
                        {
                            process_big_command();
                            geted_size = 0;
                            tout_cnt = 0;
                        }
                        break;
                     case 5:
                        ShowData("ROM E 0000h 256\r\n");
                        data_port.WriteLine("ROM E 0000h 256\r\n");
                        trans_function = 6;
                        read_size = 5;
                        break;
                     case 6:
                        if (geted_size == read_size)
                        {
                            trans_function = 0;
                            read_size = 2;
                            geted_size = 0;
                        }
                        break;
                }
                
            }
        }
        byte calulate_sent_crc(byte[] ary, int sz)
        {
            byte data = 0 ;
            for(int i = 0 ; i < sz ; i++ ){
                data ^= ary[i] ;
            }
            return data ;
        }

        void sent_data_with_crc(byte[] dat, int offset, int sz)
        {
            int i ;
            byte[] ary = new byte[129];
            for ( i = 0; i < sz; i++ )
            {
                ary[i] = dat[offset + i];
            }
            ary[i] = calulate_sent_crc(ary, sz);
            data_port.Write(ary, 0, sz + 1);
        }

        void sent_first_big_data_command() 
        {
            if (sent_size >= 128)
            {
                out_size = 128;
            }
            else 
            {
                out_size = sent_size;
            }
            sent_data_with_crc(image_data, send_offset, out_size);
        }
        int show_dot_cnt = 0;
        void process_big_command()
        {
            if ((in_datas[0] == 'T') && (in_datas[1] == 'O'))
            {
                trans_function = 0;
                ShowData("TO\r\n");
                trans_function = 0;
                ShowButton( true );
                return;
            }

            if ((in_datas[0] == 'N') && (in_datas[1] == 'G'))
            {
                System.Console.Out.WriteLine("NG");
                ShowData("\r\nNG\r\n");
                trans_function = 0;
                ShowButton( true );
                return;
            }
            if ((in_datas[0] == 'O') && (in_datas[1] == 'K'))
            {
                show_dot_cnt++;
                if (show_dot_cnt >= 8)
                {
                    ShowData(".");
                    show_dot_cnt = 0;
                }
            }
                send_offset += out_size;
                if ((sent_size - send_offset) > 128)
                {
                    out_size = 128;
                }
                else if ((sent_size - send_offset) > 0)
                {
                    out_size = sent_size - send_offset;
                }
                else // send finish 
                {
                    index_of_image_list++;
					if( index_of_image_list < m_img_list.Items.Count )
					{
                        total_offset += send_offset ;
                        send_offset = 0;
                        ShowData("\r\n");
                        load_image_data();
                        trans_function = 1;
                    }
                    else
                    {
                        trans_function = 0;
                        ShowButton( true );
                        ShowData("Finish Time : " + DateTime.Now.TimeOfDay.ToString());
                }
                    return;
                }
                sent_data_with_crc(image_data, send_offset, out_size);
          //      ShowData("write offset=" + send_offset + " , sz =" + out_size );
         //   }
        }
        

        private void btn_ble_com_port_Click(object sender, EventArgs e)
        {
            int idx = cb_com_port.SelectedIndex;
            if (idx == -1) 
            {
                return;
            }
            if (data_port.IsOpen )
            {
                data_port.Close();
                btn_usb_open.Text = "Open";
            }
            else
            {
                data_port.PortName = cb_com_port.Items[idx].ToString();
                data_port.Open();
                btn_usb_open.Text = "Close";
            }

        }
		private static void AddText(FileStream fs, string value)
		{
			byte[] info = new UTF8Encoding(true).GetBytes(value);
			fs.Write(info, 0, info.Length);
		}
        int out_size , total_offset ;
        int sent_size, send_offset ;
		int index_of_image_list ;
		byte[] image_data = new byte[ 153800 ]; // 320 * 240 * 2 + 200
        private void load_image_data()
		{
            string path = txt_path.Text;
			Bitmap bmp = new Bitmap(path + "\\" + m_img_list.Items[index_of_image_list]);
            BitmapData bmpData = bmp.LockBits(new Rectangle(0, 0, bmp.Width, bmp.Height), ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
            Stopwatch sw = new Stopwatch();

            ShowData( "Image " + m_img_list.Items[index_of_image_list] + "\t");
            sw.Start();

                //一個一個byte複製
                int dataIndex = 4, height = bmpData.Height, width = bmpData.Width;
                int rgb565 ;
				image_data[0] = (byte)(( width >> 8 ) & 0xFF) ;
				image_data[1] = (byte)( width & 0xFF);
				image_data[2] = (byte)(( height >> 8 ) & 0xFF) ;
				image_data[3] = (byte)( height & 0xFF);
				
                unsafe
                {
                    byte* p = (byte*)bmpData.Scan0.ToPointer();
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            rgb565 = (((int)p[0] & 0x1F) << 11);
                            rgb565 |= (((int)p[1] & 0x3F) << 5);
                            rgb565 |= ((int)p[2] & 0x1F) ;
                            image_data[dataIndex++] = (byte)(( rgb565 >> 8 ) & 0xFF) ;
                            image_data[dataIndex++] = (byte)( rgb565 & 0xFF);
                            p += 3;
                        }
                    }
                }

            sw.Stop();          
            bmp.UnlockBits(bmpData); 
			
            sent_size = height * width * 2 + 4;
		}
		
		
        bool tell_thread_break ;
        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            tell_thread_break = true;
        }

        private void btn_erase_Click(object sender, EventArgs e)
        {
            if (!data_port.IsOpen) return;
            trans_function = 5;
        }
        private void btn_big_data_sent_Click(object sender, EventArgs e)
        {
            ShowData("Start Time : "+ DateTime.Now.TimeOfDay.ToString());
            if (!data_port.IsOpen) return;
            if( m_img_list.Items.Count == 0 )
            {
                return ;
            }
            data_port.ReadExisting();
           // data_port.DiscardOutBuffer();
            btn_big_data_sent.Visible = false;
            load_image_data(); 
            trans_function = 1;
			index_of_image_list = 0 ;
            total_offset = 0 ;
			send_offset = 0 ;
        }

        private void btn_add_pic_Click(object sender, EventArgs e)
        {
            OpenFileDialog choofdlog = new OpenFileDialog();
            choofdlog.Filter = "All Files (*.*)|*.*";
            choofdlog.FilterIndex = 1;
            choofdlog.Multiselect = true;
            if (txt_path.Text != "") {
                choofdlog.InitialDirectory = txt_path.Text;
            }
            if (choofdlog.ShowDialog() == DialogResult.OK)
            {
                foreach (string sFileName in choofdlog.SafeFileNames)
                {
                    m_img_list.Items.Add(sFileName);
                }
            }
        }

        private void btn_path_Click(object sender, EventArgs e)
        {
            OpenFileDialog folderBrowser = new OpenFileDialog();
            // Set validate names and check file exists to false otherwise windows will
            // not let you select "Folder Selection."
            folderBrowser.ValidateNames = false;
            folderBrowser.CheckFileExists = false;
            folderBrowser.CheckPathExists = true;
            // Always default to Folder Selection.
            folderBrowser.FileName = "Folder Selection.";
            if (folderBrowser.ShowDialog() == DialogResult.OK)
            {
                 txt_path.Text  = Path.GetDirectoryName(folderBrowser.FileName);
            }
        }

        private void export_head_data(string fpath)
        {
            string path = txt_path.Text;
            using (FileStream fs = new FileStream(fpath, FileMode.Create, FileAccess.Write))
            {
                AddText(fs, "#ifndef _GUI_IMAGE_OFFSET_H_\r\n");
                AddText(fs, "#define _GUI_IMAGE_OFFSET_H_\r\n");
                int n_offset = 0;
                for (int i = 0; i < m_img_list.Items.Count; i++)
                {
                    string fname = m_img_list.Items[i].ToString();
                    Bitmap bmp = new Bitmap(path + "\\" + fname);
                    string def_name = fname.ToUpper();
                    def_name = def_name.Replace('.','_');
                    AddText(fs, "#define " + def_name + "\t" + n_offset + "\r\n");
                    n_offset += bmp.Height * bmp.Width * 2 + 4;
                }

                AddText(fs, "#define END_ADDRESS\t" + n_offset + "\r\n");

                AddText(fs, "#endif\r\n");
                fs.Close();
            }
        }
        private void btn_export_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.Filter = "head file|*.h";
            saveFileDialog1.Title = "Save head File";
            saveFileDialog1.ShowDialog();

            // If the file name is not an empty string open it for saving.
            if (saveFileDialog1.FileName != "")
            {
                export_head_data(saveFileDialog1.FileName);
            }
        }


        private delegate void ShowButtonDelegate(bool visable);
        private void ShowButton(bool visable)
        {
            if (cmd_res.InvokeRequired)
            {
                ShowButtonDelegate d = ShowButton;
                cmd_res.BeginInvoke(d, visable);
            }
            else
            {
                btn_big_data_sent.Visible = visable;
            }
        }

        private delegate void DoDataDelegate(string lines);

        private void btn_del_pic_Click(object sender, EventArgs e)
        {
            for (int i = this.m_img_list.Items.Count - 1; i >= 0; i--)
            {
                this.m_img_list.Items.Remove(this.m_img_list.SelectedItem);
            }
        }

        private void btn_down_Click(object sender, EventArgs e)
        {
            int index = m_img_list.SelectedIndex;

            if (index == -1) //没选中
            {
                return;
            }
            else if (index < m_img_list.Items.Count - 1)
            {
                string str = m_img_list.Items[index].ToString();
                m_img_list.Items.RemoveAt(index);
                m_img_list.Items.Insert(index + 1, str);
                m_img_list.SelectedIndex = index + 1;
            }
        }

        private void btn_up_Click(object sender, EventArgs e)
        {
            int index = m_img_list.SelectedIndex;

            if (index == -1)//没选中
            {
                return;
            }
            else if (index > 0)
            {
                string str = m_img_list.Items[index].ToString();
                m_img_list.Items.RemoveAt(index);
                m_img_list.Items.Insert(index - 1, str);
                m_img_list.SelectedIndex = index - 1;
            }
        }

        private void btn_save_setting_Click(object sender, EventArgs e)
        {

        }

        private void ShowData(string lines)
        {
            if (cmd_res.InvokeRequired)
            {
                DoDataDelegate d = ShowData;
                cmd_res.BeginInvoke(d, lines);
            }
            else
            {
                cmd_res.Text += lines ;
            }
        }

        private void btn_clear_Click(object sender, EventArgs e)
        {
            cmd_res.Text = "";
        }

    }
}
