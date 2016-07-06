#include "util/format.h"
#include "util/realsense_formatters.h"
#include <pxcsensemanager.h>
#include <wx/wx.h>
#include <wx/rawbmp.h>
#include <wx/dcbuffer.h>
#include <map>
#include <mutex>
#include <cstdint>

static void Error(wxWindow *parent, const std::string &message)
{
  wxMessageDialog msg(parent, wxString(message), wxT("Error"), wxOK | wxCENTER | wxICON_ERROR);
  msg.ShowModal();
}

class MyPanel : public wxPanel
{
private:
  wxBitmap m_bmp{ 640, 480, 24 }; // must be 24-bit to access with wxNativePixelData

public:
  MyPanel(wxFrame *parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(640, 480))
  {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
  }

  wxBitmap & GetBitmap()
  {
    return m_bmp;
  }

  void PaintEvent(wxPaintEvent &evt)
  {
    wxAutoBufferedPaintDC dc(this);
    Render(dc);
  }

  void PaintNow()
  {
    wxClientDC dc(this);
    Render(dc);
  }

  void Render(wxDC &dc)
  {
    dc.DrawBitmap(m_bmp, 0, 0);
  }
  DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(MyPanel, wxPanel)
EVT_PAINT(MyPanel::PaintEvent)
END_EVENT_TABLE()

class MyApp : public wxApp
{
private:
  wxFrame *m_frame;
  MyPanel *m_color_panel;
  MyPanel *m_depth_panel;
  PXCSenseManager *m_sense_mgr;
  bool m_render_loop_active = false;

public:
  void DrawColorFrame(wxBitmap *bmp, PXCImage *color)
  {
    PXCImage::ImageInfo info = color->QueryInfo();
    PXCImage::ImageData data;
    pxcStatus status = color->AcquireAccess(PXCImage::Access::ACCESS_READ, PXCImage::PixelFormat::PIXEL_FORMAT_RGB24, &data);
    if (PXC_STATUS_NO_ERROR != status)
      m_frame->SetStatusText(std::string(Util::Format() << status));
    else
    {
      wxNativePixelData pixels(*bmp);
      if (!pixels || bmp->GetWidth() != info.width || bmp->GetHeight() != info.height || data.pitches[0] != info.width * 3)
        return; //TODO: display an error if could not get pixels or if image is not of expected type
      auto p = pixels.GetPixels();
      int i = 0;
      for (int y = 0; y < info.height; y++)
      {
        auto row_start = p;
        for (int x = 0; x < info.width; x++)
        {
          p.Blue() = data.planes[0][i + 0];
          p.Green() = data.planes[0][i + 1];
          p.Red() = data.planes[0][i + 2];
          i += 3;
          ++p;
        }
        p = row_start;
        p.OffsetY(pixels, 1);
      }
    }
  }

  //TODO: templatize
  void DrawDepthFrame(wxBitmap *bmp, PXCImage *depth)
  {
    PXCImage::ImageInfo info = depth->QueryInfo();
    PXCImage::ImageData data;
    pxcStatus status = depth->AcquireAccess(PXCImage::Access::ACCESS_READ, PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH, &data);
    if (PXC_STATUS_NO_ERROR != status)
      m_frame->SetStatusText(std::string(Util::Format() << status));
    else
    {
      wxNativePixelData pixels(*bmp);
      if (!pixels || bmp->GetWidth() != info.width || bmp->GetHeight() != info.height || data.pitches[0] != info.width * 2)
        return; //TODO: display an error if could not get pixels or if image is not of expected type
      auto p = pixels.GetPixels();
      int i = 0;
      for (int y = 0; y < info.height; y++)
      {
        auto row_start = p;
        for (int x = 0; x < info.width; x++)
        {
          uint16_t z_raw = uint16_t(data.planes[0][i + 0] << 8) | data.planes[0][i + 1];
          //uint16_t z_raw = *(uint16_t *) &(data.planes[0][i]);
          uint8_t z = uint8_t(255.0f * float(z_raw) / float(0xffff));
          p.Blue() = z;
          p.Green() = z;
          p.Red() = z;
          i += 2;
          ++p;
        }
        p = row_start;
        p.OffsetY(pixels, 1);
      }
    }
  }

  void OnIdle(wxIdleEvent &evt)
  {
    if (!m_render_loop_active)
      return;
    if (m_sense_mgr->AcquireFrame(true) != PXC_STATUS_NO_ERROR)
      goto do_nothing;
    PXCCapture::Sample *sample = m_sense_mgr->QuerySample();
    DrawColorFrame(&m_color_panel->GetBitmap(), sample->color);
    DrawDepthFrame(&m_depth_panel->GetBitmap(), sample->depth);
    m_sense_mgr->ReleaseFrame();
    static int frame = 0;
    //m_frame->SetStatusText((std::string) (Util::Format() << "Frame: " << frame++));
    m_color_panel->Refresh(false);
    m_depth_panel->Refresh(false);
  do_nothing:
    evt.RequestMore();
  }

  void ActivateRenderLoop(bool on)
  {
    //TODO: create MyFrame inherited from wxFrame and catch OnClose() (https://wiki.wxwidgets.org/Making_a_render_loop)
    if (on && !m_render_loop_active)
    {
      Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(MyApp::OnIdle));
      m_render_loop_active = true;
    }
    else if (!on && m_render_loop_active)
    {
      Disconnect(wxEVT_IDLE, wxIdleEventHandler(MyApp::OnIdle));
      m_render_loop_active = false;
    }
  }

  bool InitRealSense()
  {
    m_sense_mgr = PXCSenseManager::CreateInstance();
    if (!m_sense_mgr)
    {
      Error(m_frame, "Unable to initialize Intel RealSense camera.");
      return false;
    }
    m_sense_mgr->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 640, 480, 30);
    m_sense_mgr->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, 640, 480, 30);
    pxcStatus status = m_sense_mgr->Init();
    if (PXC_STATUS_NO_ERROR != status)
    {
      Error(m_frame, Util::Format() << status);
      return false;
    }
    return true;
  }

  virtual bool OnInit() wxOVERRIDE
  {
    if (!wxApp::OnInit())
      return false;
    m_frame = new wxFrame(0, -1, wxT("RealSense Capture"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER & ~wxMAXIMIZE_BOX);
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    m_color_panel = new MyPanel(m_frame);
    m_depth_panel = new MyPanel(m_frame);
    sizer->Add(m_color_panel, 1, wxEXPAND);
    sizer->Add(m_depth_panel, 1, wxEXPAND);
    m_frame->CreateStatusBar();
    m_frame->SetSizer(sizer);
    m_frame->SetAutoLayout(true);
    m_frame->Fit(); // setting size in wxFrame() ctor doesn't seem to work
    m_frame->Show(true);
    if (!InitRealSense())
      return false;
    ActivateRenderLoop(true);
    return true;
  }

  virtual int OnExit() wxOVERRIDE
  {
    m_sense_mgr->Release();
    return 0;
  }
};

wxIMPLEMENT_APP(MyApp);
