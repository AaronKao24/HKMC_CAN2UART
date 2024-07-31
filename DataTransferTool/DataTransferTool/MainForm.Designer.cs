namespace DataTransferTool
{
    partial class MainForm
    {
        /// <summary>
        /// 設計工具所需的變數。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清除任何使用中的資源。
        /// </summary>
        /// <param name="disposing">如果應該處置 Managed 資源則為 true，否則為 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 設計工具產生的程式碼

        /// <summary>
        /// 此為設計工具支援所需的方法 - 請勿使用程式碼編輯器
        /// 修改這個方法的內容。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.data_port = new System.IO.Ports.SerialPort(this.components);
            this.lab_COM = new System.Windows.Forms.Label();
            this.cb_com_port = new System.Windows.Forms.ComboBox();
            this.btn_usb_open = new System.Windows.Forms.Button();
            this.btn_export = new System.Windows.Forms.Button();
            this.btn_save_setting = new System.Windows.Forms.Button();
            this.btn_big_data_sent = new System.Windows.Forms.Button();
            this.num_offset = new System.Windows.Forms.NumericUpDown();
            this.lab_offset = new System.Windows.Forms.Label();
            this.cmd_res = new System.Windows.Forms.TextBox();
            this.lab_res = new System.Windows.Forms.Label();
            this.m_img_list = new System.Windows.Forms.ListBox();
            this.btn_add_pic = new System.Windows.Forms.Button();
            this.btn_del_pic = new System.Windows.Forms.Button();
            this.btn_down = new System.Windows.Forms.Button();
            this.btn_up = new System.Windows.Forms.Button();
            this.txt_path = new System.Windows.Forms.TextBox();
            this.btn_path = new System.Windows.Forms.Button();
            this.btn_clear = new System.Windows.Forms.Button();
            this.btn_erase = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.num_offset)).BeginInit();
            this.SuspendLayout();
            // 
            // data_port
            // 
            this.data_port.BaudRate = 115200;
            this.data_port.ReadBufferSize = 256;
            this.data_port.ReadTimeout = 50;
            this.data_port.WriteBufferSize = 256;
            // 
            // lab_COM
            // 
            this.lab_COM.AutoSize = true;
            this.lab_COM.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lab_COM.Location = new System.Drawing.Point(434, 132);
            this.lab_COM.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.lab_COM.Name = "lab_COM";
            this.lab_COM.Size = new System.Drawing.Size(110, 29);
            this.lab_COM.TabIndex = 49;
            this.lab_COM.Text = "USB PORT";
            // 
            // cb_com_port
            // 
            this.cb_com_port.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cb_com_port.Font = new System.Drawing.Font("新細明體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cb_com_port.FormattingEnabled = true;
            this.cb_com_port.Location = new System.Drawing.Point(554, 128);
            this.cb_com_port.Margin = new System.Windows.Forms.Padding(4);
            this.cb_com_port.Name = "cb_com_port";
            this.cb_com_port.Size = new System.Drawing.Size(156, 28);
            this.cb_com_port.TabIndex = 48;
            // 
            // btn_usb_open
            // 
            this.btn_usb_open.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btn_usb_open.Location = new System.Drawing.Point(718, 128);
            this.btn_usb_open.Margin = new System.Windows.Forms.Padding(4);
            this.btn_usb_open.Name = "btn_usb_open";
            this.btn_usb_open.Size = new System.Drawing.Size(82, 35);
            this.btn_usb_open.TabIndex = 47;
            this.btn_usb_open.Text = "Open";
            this.btn_usb_open.UseVisualStyleBackColor = true;
            this.btn_usb_open.Click += new System.EventHandler(this.btn_ble_com_port_Click);
            // 
            // btn_export
            // 
            this.btn_export.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btn_export.Location = new System.Drawing.Point(437, 64);
            this.btn_export.Margin = new System.Windows.Forms.Padding(4);
            this.btn_export.Name = "btn_export";
            this.btn_export.Size = new System.Drawing.Size(173, 35);
            this.btn_export.TabIndex = 50;
            this.btn_export.Text = "Export Head File";
            this.btn_export.UseVisualStyleBackColor = true;
            this.btn_export.Click += new System.EventHandler(this.btn_export_Click);
            // 
            // btn_save_setting
            // 
            this.btn_save_setting.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btn_save_setting.Location = new System.Drawing.Point(331, 309);
            this.btn_save_setting.Margin = new System.Windows.Forms.Padding(4);
            this.btn_save_setting.Name = "btn_save_setting";
            this.btn_save_setting.Size = new System.Drawing.Size(90, 47);
            this.btn_save_setting.TabIndex = 53;
            this.btn_save_setting.Text = "Save";
            this.btn_save_setting.UseVisualStyleBackColor = true;
            this.btn_save_setting.Click += new System.EventHandler(this.btn_save_setting_Click);
            // 
            // btn_big_data_sent
            // 
            this.btn_big_data_sent.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btn_big_data_sent.Location = new System.Drawing.Point(650, 216);
            this.btn_big_data_sent.Margin = new System.Windows.Forms.Padding(4);
            this.btn_big_data_sent.Name = "btn_big_data_sent";
            this.btn_big_data_sent.Size = new System.Drawing.Size(150, 35);
            this.btn_big_data_sent.TabIndex = 57;
            this.btn_big_data_sent.Text = "Sent To Module";
            this.btn_big_data_sent.UseVisualStyleBackColor = true;
            this.btn_big_data_sent.Click += new System.EventHandler(this.btn_big_data_sent_Click);
            // 
            // num_offset
            // 
            this.num_offset.Font = new System.Drawing.Font("新細明體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.num_offset.Location = new System.Drawing.Point(521, 218);
            this.num_offset.Margin = new System.Windows.Forms.Padding(4);
            this.num_offset.Maximum = new decimal(new int[] {
            2097151,
            0,
            0,
            0});
            this.num_offset.Name = "num_offset";
            this.num_offset.Size = new System.Drawing.Size(112, 31);
            this.num_offset.TabIndex = 60;
            // 
            // lab_offset
            // 
            this.lab_offset.AutoSize = true;
            this.lab_offset.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lab_offset.Location = new System.Drawing.Point(436, 218);
            this.lab_offset.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.lab_offset.Name = "lab_offset";
            this.lab_offset.Size = new System.Drawing.Size(69, 29);
            this.lab_offset.TabIndex = 62;
            this.lab_offset.Text = "offset";
            // 
            // cmd_res
            // 
            this.cmd_res.Location = new System.Drawing.Point(436, 281);
            this.cmd_res.Margin = new System.Windows.Forms.Padding(4);
            this.cmd_res.Multiline = true;
            this.cmd_res.Name = "cmd_res";
            this.cmd_res.ReadOnly = true;
            this.cmd_res.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.cmd_res.Size = new System.Drawing.Size(487, 267);
            this.cmd_res.TabIndex = 65;
            // 
            // lab_res
            // 
            this.lab_res.AutoSize = true;
            this.lab_res.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lab_res.Location = new System.Drawing.Point(434, 251);
            this.lab_res.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.lab_res.Name = "lab_res";
            this.lab_res.Size = new System.Drawing.Size(74, 29);
            this.lab_res.TabIndex = 66;
            this.lab_res.Text = "Result";
            // 
            // m_img_list
            // 
            this.m_img_list.FormattingEnabled = true;
            this.m_img_list.ItemHeight = 20;
            this.m_img_list.Location = new System.Drawing.Point(16, 38);
            this.m_img_list.Margin = new System.Windows.Forms.Padding(4);
            this.m_img_list.Name = "m_img_list";
            this.m_img_list.ScrollAlwaysVisible = true;
            this.m_img_list.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.m_img_list.Size = new System.Drawing.Size(307, 504);
            this.m_img_list.TabIndex = 67;
            // 
            // btn_add_pic
            // 
            this.btn_add_pic.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btn_add_pic.Location = new System.Drawing.Point(331, 64);
            this.btn_add_pic.Margin = new System.Windows.Forms.Padding(4);
            this.btn_add_pic.Name = "btn_add_pic";
            this.btn_add_pic.Size = new System.Drawing.Size(90, 38);
            this.btn_add_pic.TabIndex = 68;
            this.btn_add_pic.Text = "ADD";
            this.btn_add_pic.UseVisualStyleBackColor = true;
            this.btn_add_pic.Click += new System.EventHandler(this.btn_add_pic_Click);
            // 
            // btn_del_pic
            // 
            this.btn_del_pic.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btn_del_pic.Location = new System.Drawing.Point(331, 110);
            this.btn_del_pic.Margin = new System.Windows.Forms.Padding(4);
            this.btn_del_pic.Name = "btn_del_pic";
            this.btn_del_pic.Size = new System.Drawing.Size(90, 38);
            this.btn_del_pic.TabIndex = 69;
            this.btn_del_pic.Text = "DEL";
            this.btn_del_pic.UseVisualStyleBackColor = true;
            this.btn_del_pic.Click += new System.EventHandler(this.btn_del_pic_Click);
            // 
            // btn_down
            // 
            this.btn_down.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btn_down.Location = new System.Drawing.Point(331, 223);
            this.btn_down.Margin = new System.Windows.Forms.Padding(4);
            this.btn_down.Name = "btn_down";
            this.btn_down.Size = new System.Drawing.Size(90, 38);
            this.btn_down.TabIndex = 71;
            this.btn_down.Text = "DOWN";
            this.btn_down.UseVisualStyleBackColor = true;
            this.btn_down.Click += new System.EventHandler(this.btn_down_Click);
            // 
            // btn_up
            // 
            this.btn_up.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btn_up.Location = new System.Drawing.Point(331, 177);
            this.btn_up.Margin = new System.Windows.Forms.Padding(4);
            this.btn_up.Name = "btn_up";
            this.btn_up.Size = new System.Drawing.Size(90, 38);
            this.btn_up.TabIndex = 70;
            this.btn_up.Text = "UP";
            this.btn_up.UseVisualStyleBackColor = true;
            this.btn_up.Click += new System.EventHandler(this.btn_up_Click);
            // 
            // txt_path
            // 
            this.txt_path.Location = new System.Drawing.Point(16, 6);
            this.txt_path.Name = "txt_path";
            this.txt_path.ReadOnly = true;
            this.txt_path.Size = new System.Drawing.Size(830, 31);
            this.txt_path.TabIndex = 73;
            // 
            // btn_path
            // 
            this.btn_path.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btn_path.Location = new System.Drawing.Point(849, 6);
            this.btn_path.Margin = new System.Windows.Forms.Padding(4);
            this.btn_path.Name = "btn_path";
            this.btn_path.Size = new System.Drawing.Size(65, 35);
            this.btn_path.TabIndex = 74;
            this.btn_path.Text = "Path";
            this.btn_path.UseVisualStyleBackColor = true;
            this.btn_path.Click += new System.EventHandler(this.btn_path_Click);
            // 
            // btn_clear
            // 
            this.btn_clear.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btn_clear.Location = new System.Drawing.Point(331, 507);
            this.btn_clear.Margin = new System.Windows.Forms.Padding(4);
            this.btn_clear.Name = "btn_clear";
            this.btn_clear.Size = new System.Drawing.Size(75, 35);
            this.btn_clear.TabIndex = 75;
            this.btn_clear.Text = "Clear";
            this.btn_clear.UseVisualStyleBackColor = true;
            this.btn_clear.Click += new System.EventHandler(this.btn_clear_Click);
            // 
            // btn_erase
            // 
            this.btn_erase.Font = new System.Drawing.Font("Calibri", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btn_erase.Location = new System.Drawing.Point(436, 168);
            this.btn_erase.Margin = new System.Windows.Forms.Padding(4);
            this.btn_erase.Name = "btn_erase";
            this.btn_erase.Size = new System.Drawing.Size(90, 35);
            this.btn_erase.TabIndex = 76;
            this.btn_erase.Text = "Erase";
            this.btn_erase.UseVisualStyleBackColor = true;
            this.btn_erase.Click += new System.EventHandler(this.btn_erase_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(10F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(936, 561);
            this.Controls.Add(this.btn_erase);
            this.Controls.Add(this.btn_clear);
            this.Controls.Add(this.btn_path);
            this.Controls.Add(this.txt_path);
            this.Controls.Add(this.btn_down);
            this.Controls.Add(this.btn_up);
            this.Controls.Add(this.btn_del_pic);
            this.Controls.Add(this.btn_add_pic);
            this.Controls.Add(this.m_img_list);
            this.Controls.Add(this.lab_res);
            this.Controls.Add(this.cmd_res);
            this.Controls.Add(this.lab_offset);
            this.Controls.Add(this.num_offset);
            this.Controls.Add(this.btn_big_data_sent);
            this.Controls.Add(this.btn_save_setting);
            this.Controls.Add(this.btn_export);
            this.Controls.Add(this.lab_COM);
            this.Controls.Add(this.cb_com_port);
            this.Controls.Add(this.btn_usb_open);
            this.Font = new System.Drawing.Font("新細明體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "MainForm";
            this.Text = "Data Transfer";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.num_offset)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.IO.Ports.SerialPort data_port;
        private System.Windows.Forms.Label lab_COM;
        private System.Windows.Forms.ComboBox cb_com_port;
        private System.Windows.Forms.Button btn_usb_open;
        private System.Windows.Forms.Button btn_export;
        private System.Windows.Forms.Button btn_save_setting;
        private System.Windows.Forms.Button btn_big_data_sent;
        private System.Windows.Forms.NumericUpDown num_offset;
        private System.Windows.Forms.Label lab_offset;
        private System.Windows.Forms.TextBox cmd_res;
        private System.Windows.Forms.Label lab_res;
        private System.Windows.Forms.ListBox m_img_list;
        private System.Windows.Forms.Button btn_add_pic;
        private System.Windows.Forms.Button btn_del_pic;
        private System.Windows.Forms.Button btn_down;
        private System.Windows.Forms.Button btn_up;
        private System.Windows.Forms.TextBox txt_path;
        private System.Windows.Forms.Button btn_path;
        private System.Windows.Forms.Button btn_clear;
        private System.Windows.Forms.Button btn_erase;
    }
}

