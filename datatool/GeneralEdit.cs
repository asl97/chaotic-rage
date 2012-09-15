﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using SharpGL;

namespace datatool
{
    public partial class GeneralEdit : Form
    {
        private List<base_item> items;
        private string title;
        private Type type;

        private int previewType;
        private OpenGLControl glPreviewCtl;
        private float rotation;
        private PreviewBase preview;


        public GeneralEdit(List<base_item> items, string title, Type type, PreviewBase preview = null)
        {
            InitializeComponent();
            this.items = items;
            this.title = title;
            this.type = type;

            if (preview != null) {
                this.preview = preview;
                this.setPreviewType(3);
            } else {
                this.nukePreview();
                this.toolPreview.Visible = false;
            }
        }
        
        private void setPreviewType(int t) {
            if (t == previewType) return;
            if (previewType == 0) initPreview();
            if (t == 0) nukePreview();
            previewType = t;
        }

        private void initPreview() {
            glPreviewCtl = new OpenGLControl();

            ((System.ComponentModel.ISupportInitialize)(this.glPreviewCtl)).BeginInit();

            glPreviewCtl.Dock = DockStyle.Fill;
            glPreviewCtl.BitDepth = 24;
            glPreviewCtl.Resized += new System.EventHandler(this.preview_Resized);
            glPreviewCtl.OpenGLDraw += new System.Windows.Forms.PaintEventHandler(this.preview_OpenGLDraw);
            glPreviewCtl.RenderContextType = RenderContextType.FBO;
            glPreviewCtl.FrameRate = 10;
            panPreview.Controls.Add(glPreviewCtl);
            panPreview.Visible = true;

            ((System.ComponentModel.ISupportInitialize)(this.glPreviewCtl)).EndInit();
        }

        private void preview_Resized(object sender, EventArgs e)
        {
            OpenGL gl = glPreviewCtl.OpenGL;

            gl.MatrixMode(OpenGL.GL_PROJECTION);
            gl.LoadIdentity();
            gl.Perspective(60.0f, (double)glPreviewCtl.Width / (double)glPreviewCtl.Height, 0.01, 100.0);
            gl.LookAt(-5, 5, -5, 0, 0, 0, 0, 1, 0);
            gl.MatrixMode(OpenGL.GL_MODELVIEW);
        }

        private void nukePreview()
        {
            panPreview.Visible = false;
        }

        private void GeneralEdit_Load(object sender, EventArgs e)
        {
            this.Text = this.title;
            list.Clear();
            foreach (base_item i in items) {
                list.Items.Add(new ExtraListItem(i));
            }
        }

        private void list_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (this.preview != null) {
                this.preview.Nuke(glPreviewCtl.OpenGL);
            }

            if (list.SelectedItems.Count == 0) {
                grid.SelectedObject = null;
                this.Text = this.title;
                return;
            }

            ExtraListItem listitem = (ExtraListItem)list.SelectedItems[0];
            grid.SelectedObject = listitem.Extra;
            this.Text = this.title + " - " + listitem.Extra.getName();

            if (this.preview != null) {
                this.preview.Init(glPreviewCtl.OpenGL, listitem.Extra);
            }
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            base_item item = (base_item) type.GetConstructor(new Type[0]).Invoke(new Object[0]);

            ExtraListItem listitem = new ExtraListItem(item);
            items.Add(item);
            list.Items.Add(listitem);
            listitem.Selected = true;
            grid.Focus();
        }

        private void grid_PropertyValueChanged(object s, PropertyValueChangedEventArgs e)
        {
            if (list.SelectedItems.Count == 0) {
                return;
            }

            ExtraListItem listitem = (ExtraListItem) list.SelectedItems[0];
            listitem.Text = listitem.Extra.getName();
            this.Text = this.title + " - " + listitem.Extra.getName();
        }

        private void btnRemove_Click(object sender, EventArgs e)
        {
            if (list.SelectedItems.Count == 0) {
                return;
            }

            ExtraListItem listitem = (ExtraListItem)list.SelectedItems[0];
            list.SelectedItems.Clear();

            items.Remove(listitem.Extra);
            list.Items.Remove(listitem);
        }

        private void preview_OpenGLDraw(object sender, EventArgs e)
        {
            if (this.preview == null) return;

            OpenGL gl = glPreviewCtl.OpenGL;
            
            gl.Clear(OpenGL.GL_COLOR_BUFFER_BIT | OpenGL.GL_DEPTH_BUFFER_BIT);
            gl.LoadIdentity();
            gl.Scale(1.0f, -1.0f, 1.0f);
            gl.Rotate(rotation, 0.0f, 1.0f, 0.0f);
            gl.Rotate(90, 1.0f, 0.0f, 0.0f);

            if (grid.SelectedObject == null) return;

            switch (previewType) {
                case 1:
                    gl.Color(0.7f, 0.7f, 0.7f, 1.0f);
                    gl.PolygonMode(OpenGL.GL_FRONT_AND_BACK, OpenGL.GL_LINE);
                    gl.Disable(OpenGL.GL_TEXTURE_2D);
                    preview.Render(gl);
                    break;

                case 2:
                    gl.Color(0.3f, 0.3f, 0.3f, 1.0f);
                    gl.PolygonMode(OpenGL.GL_FRONT_AND_BACK, OpenGL.GL_FILL);
                    gl.Disable(OpenGL.GL_TEXTURE_2D);
                    preview.Render(gl);
                    gl.Color(0.7f, 0.7f, 0.7f, 1.0f);
                    gl.PolygonMode(OpenGL.GL_FRONT_AND_BACK, OpenGL.GL_LINE);
                    gl.LineWidth(1.3f);
                    preview.Render(gl);
                    break;

                case 3:
                    gl.PolygonMode(OpenGL.GL_FRONT_AND_BACK, OpenGL.GL_FILL);
                    gl.Enable(OpenGL.GL_TEXTURE_2D);
                    preview.Render(gl);
                    break;
            }

            rotation += 0.3f;
        }

        private void previewOff_Click(object sender, EventArgs e)
        {
            foreach (ToolStripMenuItem b in toolPreview.DropDown.Items) { b.Checked = false; }
            previewOff.Checked = true;

            setPreviewType(0);
        }

        private void previewWireframe_Click(object sender, EventArgs e)
        {
            foreach (ToolStripMenuItem b in toolPreview.DropDown.Items) { b.Checked = false; }
            previewWireframe.Checked = true;

            setPreviewType(1);
        }

        private void previewSolid_Click(object sender, EventArgs e)
        {
            foreach (ToolStripMenuItem b in toolPreview.DropDown.Items) { b.Checked = false; }
            previewSolid.Checked = true;

            setPreviewType(2);
        }

        private void previewTextured_Click(object sender, EventArgs e)
        {
            foreach (ToolStripMenuItem b in toolPreview.DropDown.Items) { b.Checked = false; }
            previewTextured.Checked = true;

            setPreviewType(3);
        }
    }
}