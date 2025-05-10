#include <wx/wx.h>
#include <wx/grid.h>
#include <vector>
#include <algorithm>
#include <string>

// --- Структуры ---
struct Job {
    int id;
    std::string name;
    int duration;
    int priority;
    int deadline;
};

struct ScheduledJob {
    Job job;
    int startTime;
    int endTime;
};

// --- Планировщик ---
class Scheduler {
public:
    enum class Mode {
        ByPriority,
        ShortestJobFirst,
        EarliestDeadlineFirst,
        FirstComeFirstServed
    };

    static std::vector<ScheduledJob> Generate(std::vector<Job> jobs, Mode mode) {
        switch (mode) {
            case Mode::ByPriority:
                std::sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) {
                    return a.priority > b.priority;
                });
                break;
            case Mode::ShortestJobFirst:
                std::sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) {
                    return a.duration < b.duration;
                });
                break;
            case Mode::EarliestDeadlineFirst:
                std::sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) {
                    return a.deadline < b.deadline;
                });
                break;
            case Mode::FirstComeFirstServed:
                std::sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) {
                    return a.id < b.id;
                });
                break;
        }

        std::vector<ScheduledJob> schedule;
        int currentTime = 0;
        for (const auto& job : jobs) {
            schedule.push_back({job, currentTime, currentTime + job.duration});
            currentTime += job.duration;
        }
        return schedule;
    }
};

// --- Главное окно ---
class MyFrame : public wxFrame {
public:
    MyFrame() : wxFrame(nullptr, wxID_ANY, "Workshop Scheduler", wxDefaultPosition, wxSize(900, 600)) {
        wxPanel* panel = new wxPanel(this);

        // --- Layout ---
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        // Таблица
        grid = new wxGrid(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
        grid->CreateGrid(0, 5);
        grid->SetColLabelValue(0, "ID");
        grid->SetColLabelValue(1, "Name");
        grid->SetColLabelValue(2, "Duration");
        grid->SetColLabelValue(3, "Priority");
        grid->SetColLabelValue(4, "Deadline");
        grid->SetMinSize(wxSize(850, 250));

        mainSizer->Add(grid, 0, wxALL | wxEXPAND, 10);

        // Кнопки и выбор алгоритма
        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

        wxButton* btnAddRow = new wxButton(panel, wxID_ANY, "Add Row");
        wxButton* btnDeleteRow = new wxButton(panel, wxID_ANY, "Delete Row");
        wxButton* btnSchedule = new wxButton(panel, wxID_ANY, "Run Scheduling");

        algoChoice = new wxChoice(panel, wxID_ANY);
        algoChoice->Append("By Priority");
        algoChoice->Append("Shortest Job First");
        algoChoice->Append("Earliest Deadline First");
        algoChoice->Append("First Come First Served");
        algoChoice->SetSelection(0);

        buttonSizer->Add(btnAddRow, 0, wxALL, 5);
        buttonSizer->Add(btnDeleteRow, 0, wxALL, 5);
        buttonSizer->Add(btnSchedule, 0, wxALL, 5);
        buttonSizer->AddStretchSpacer();
        buttonSizer->Add(new wxStaticText(panel, wxID_ANY, "Algorithm:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        buttonSizer->Add(algoChoice, 0, wxALL, 5);

        mainSizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

        // Результат
        output = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 200),
                                wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
        output->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        mainSizer->Add(output, 1, wxALL | wxEXPAND, 10);

        panel->SetSizer(mainSizer);

        // --- Bindings ---
        btnAddRow->Bind(wxEVT_BUTTON, &MyFrame::OnAddRow, this);
        btnDeleteRow->Bind(wxEVT_BUTTON, &MyFrame::OnDeleteRow, this);
        btnSchedule->Bind(wxEVT_BUTTON, &MyFrame::OnScheduleClicked, this);
    }

private:
    wxGrid* grid;
    wxChoice* algoChoice;
    wxTextCtrl* output;

    void OnAddRow(wxCommandEvent&) {
        grid->AppendRows(1);
    }

    void OnDeleteRow(wxCommandEvent&) {
        wxArrayInt selectedRows = grid->GetSelectedRows();
        int rowToDelete = selectedRows.IsEmpty() ? grid->GetNumberRows() - 1 : selectedRows[0];

        if (rowToDelete >= 0 && rowToDelete < grid->GetNumberRows()) {
            grid->DeleteRows(rowToDelete, 1);
        } else {
            wxMessageBox("No row selected or available.", "Warning", wxICON_WARNING);
        }
    }

    void OnScheduleClicked(wxCommandEvent&) {
        std::vector<Job> jobs;
        int rows = grid->GetNumberRows();

        for (int i = 0; i < rows; ++i) {
            try {
                int id = std::stoi(grid->GetCellValue(i, 0).ToStdString());
                std::string name = grid->GetCellValue(i, 1).ToStdString();
                int duration = std::stoi(grid->GetCellValue(i, 2).ToStdString());
                int priority = std::stoi(grid->GetCellValue(i, 3).ToStdString());
                int deadline = std::stoi(grid->GetCellValue(i, 4).ToStdString());

                jobs.push_back({id, name, duration, priority, deadline});
            } catch (...) {
                wxMessageBox(wxString::Format("Invalid input in row %d", i + 1), "Error", wxICON_ERROR);
                return;
            }
        }

        Scheduler::Mode mode = static_cast<Scheduler::Mode>(algoChoice->GetSelection());
        auto schedule = Scheduler::Generate(jobs, mode);

        output->Clear();
        for (const auto& job : schedule) {
            output->AppendText(wxString::Format("Job: %-10s | Start: %2d | End: %2d\n",
                job.job.name, job.startTime, job.endTime));
        }
    }
};

// --- Приложение ---
class MyApp : public wxApp {
public:
    bool OnInit() override {
        MyFrame* frame = new MyFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
