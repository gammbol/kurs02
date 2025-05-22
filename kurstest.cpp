#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/spinctrl.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include <vector>
#include <string>
#include <algorithm>



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
    int machineId;
};

struct Machine {
    int id;
    int availableTime = 0; // Время, когда машина станет свободной
};

// --- Планировщик ---
class Scheduler {
public:
    enum Mode {
        ByPriority,
        ShortestJobFirst,
        EarliestDeadlineFirst,
        FirstComeFirstServed
    };

    static std::vector<Job> GenerateSchedule(std::vector<Job> jobs, Scheduler::Mode mode, int machineCount) {
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

        return jobs;
    }

    static std::vector<ScheduledJob> GenerateMachine(std::vector<Job> jobs, int machineCount) {
        std::vector<Machine> machines(machineCount);
        for (int i = 0; i < machineCount; ++i)
            machines[i].id = i;

        std::vector<ScheduledJob> schedule;

        for (const auto& job : jobs) {
            auto minMachine = std::min_element(machines.begin(), machines.end(),
                [](const Machine& a, const Machine& b) { return a.availableTime < b.availableTime; });

            int start = minMachine->availableTime;
            int end = start + job.duration;
            schedule.push_back({job, start, end, minMachine->id});
            minMachine->availableTime = end;
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
        grid->CreateGrid(0, 4);
        grid->SetColLabelValue(0, "Name");
        grid->SetColLabelValue(1, "Duration");
        grid->SetColLabelValue(2, "Priority");
        grid->SetColLabelValue(3, "Deadline");
        grid->SetMinSize(wxSize(850, 250));

        grid->AppendRows(1);

        mainSizer->Add(grid, 0, wxALL | wxEXPAND, 10);

        // Кнопки и выбор алгоритма
        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

        wxButton* btnAddRow = new wxButton(panel, wxID_ANY, "Add Row");
        wxButton* btnDeleteRow = new wxButton(panel, wxID_ANY, "Delete Row");
        wxButton* btnSchedule = new wxButton(panel, wxID_ANY, "Run Scheduling");

        machineCountSpin = new wxSpinCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(60, -1));
        machineCountSpin->SetRange(1, 20);
        machineCountSpin->SetValue(2); // По умолчанию 2 станка

        buttonSizer->Add(new wxStaticText(panel, wxID_ANY, "Machines:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        buttonSizer->Add(machineCountSpin, 0, wxALL, 5);

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

        menuBar = new wxMenuBar();

        menu = new wxMenu();
        menu->Append(wxID_OPEN);
        menu->AppendSeparator();
        menu->Append(wxID_EXIT);

        menuBar->Append(menu, "File");
        SetMenuBar(menuBar);

        // --- Bindings ---
        btnAddRow->Bind(wxEVT_BUTTON, &MyFrame::OnAddRow, this);
        btnDeleteRow->Bind(wxEVT_BUTTON, &MyFrame::OnDeleteRow, this);
        btnSchedule->Bind(wxEVT_BUTTON, &MyFrame::OnScheduleClicked, this);
        Bind(wxEVT_MENU, &MyFrame::OnOpenFile, this, wxID_OPEN);
        Bind(wxEVT_MENU, [](wxCommandEvent&){ wxTheApp->Exit(); }, wxID_EXIT);
    }

private:
    wxMenuBar* menuBar;
    wxMenu* menu;
    wxGrid* grid;
    wxSpinCtrl* machineCountSpin;
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
                int id = i;
                std::string name = grid->GetCellValue(i, 0).ToStdString();
                int duration = std::stoi(grid->GetCellValue(i, 1).ToStdString());
                int priority = std::stoi(grid->GetCellValue(i, 2).ToStdString());
                int deadline = std::stoi(grid->GetCellValue(i, 3).ToStdString());

                jobs.push_back({id, name, duration, priority, deadline});
            } catch (...) {
                wxMessageBox(wxString::Format("Invalid input in row %d", i + 1), "Error", wxICON_ERROR);
                return;
            }
        }

        Scheduler::Mode mode = static_cast<Scheduler::Mode>(algoChoice->GetSelection());
        int machineCount = machineCountSpin->GetValue();
        auto scheduleNoMach = Scheduler::GenerateSchedule(jobs, mode, machineCount);
        auto schedule = Scheduler::GenerateMachine(scheduleNoMach, machineCount);


        output->Clear();
        for (const auto& job : schedule) {
            output->AppendText(wxString::Format("Job: %-10s | Start: %2d | End: %2d | Machine: %d\n",
                job.job.name, job.startTime, job.endTime, job.machineId));
        }
    }

    void OnOpenFile(wxCommandEvent& event) {
         wxFileDialog openFileDialog(this, "Open .txt file", "", "",
        "TXT file (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        if (openFileDialog.ShowModal() == wxID_CANCEL)
            return;

        wxFileInputStream input(openFileDialog.GetPath());
        if (!input.IsOk()) {
            wxLogError("Не удалось открыть файл '%s'.", openFileDialog.GetPath());
            return;
        }

        wxTextInputStream text(input, "\n", wxConvUTF8);
        grid->ClearGrid();
        grid->DeleteRows(0, grid->GetNumberRows());

        int row = 0;
        while (!input.Eof()) {
            wxString line = text.ReadLine();
            wxArrayString parts = wxSplit(line, ';');

            if (parts.size() >= 4) {
                grid->AppendRows(1);
                grid->SetCellValue(row, 0, parts[0]);
                grid->SetCellValue(row, 1, parts[1]);
                grid->SetCellValue(row, 2, parts[2]);
                grid->SetCellValue(row, 3, parts[3]);
                row++;
            }
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
