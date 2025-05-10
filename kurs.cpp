// wxWidgets "Hello World" Program
 
// For compilers that support precompilation, includes "wx/wx.h".
#include "kurs.h"

 
class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};
 
wxIMPLEMENT_APP(MyApp);
 
bool MyApp::OnInit()
{
    MyFrame1 *frame = new MyFrame1(NULL);

    frame->m_choice2->

    frame->Show(true);
    return true;
}