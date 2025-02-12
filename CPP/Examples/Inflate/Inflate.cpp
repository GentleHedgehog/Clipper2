
#include <cstdlib>
#include "clipper2/clipper.h"
#include "../../Utils/clipper.svg.h"
#include "../../Utils/clipper.svg.utils.h"
#include <cmath>

#include <chrono>
#include <iostream>
#include <sstream>

namespace  {

#define PRINT_LINE(line) std::cout << line << std::endl
#define INDENTION_STRING "\n\n"

class CodeSpeed
{
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point stop;
    std::string name_;
    double lastMeasureMks;

    std::vector<double> stat_measurements_mks;
    double stat_min_mks = 0.0;
    double stat_max_mks = 0.0;
    double stat_avg_mks = 0.0;
    uint64_t stat_sum_mks = 0;
    int stat_counter = 0;
    double stat_up_threshold_mks = 0;
    int stat_up_thres_exceed_counter = 0;
    bool isMeasureStarted = false;


    void capture_start_time()
    {
        using namespace std;
        start = chrono::steady_clock::now();
    }

    void capture_stop_time_and_diff()
    {
        using namespace std;
        stop = chrono::steady_clock::now();

        auto diff = stop - start;
        lastMeasureMks = chrono::duration <double, micro> (diff).count();
    }

    void reset_stat(double up_threshold_mks = 0.0)
    {
        stat_min_mks = 1e+6 * 60.0 * 60.0; // hour
        stat_max_mks = 0.0;
        stat_avg_mks = 0.0;
        stat_sum_mks = 0;
        stat_counter = 0;
        stat_up_threshold_mks = up_threshold_mks;
        stat_up_thres_exceed_counter = 0;
        stat_measurements_mks.clear();
    }

    void calc_stat()
    {
        ++stat_counter;
        if (lastMeasureMks < stat_min_mks) stat_min_mks = lastMeasureMks;
        if (lastMeasureMks > stat_max_mks) stat_max_mks = lastMeasureMks;
        if (lastMeasureMks > stat_up_threshold_mks) ++stat_up_thres_exceed_counter;
        stat_sum_mks += lastMeasureMks;
        stat_avg_mks = stat_sum_mks / stat_counter;
        stat_measurements_mks.push_back(lastMeasureMks);
    }

    std::string printTime(double time_value_mks)
    {
        std::string ed = " mks";
        double time = time_value_mks;
        if (time_value_mks > 999999) {ed = " s"; time /= 1000000.0;}
        else if (time_value_mks > 999) {ed = " ms"; time /= 1000.0;}
        return std::to_string(time) + ed;
    }

    std::vector<std::string> form_stat_info(bool isWithAllMeasurements = false)
    {
        std::vector<std::string> str;

        str.push_back(INDENTION_STRING);
        str.push_back("================================");
        str.push_back("Time measurement statistics for " + name_);
        str.push_back("cycles: " + std::to_string(stat_counter) );
        str.push_back("all time: " + printTime(stat_sum_mks)    );
        str.push_back("max time: " + printTime(stat_max_mks)    );
        str.push_back("min time: " + printTime(stat_min_mks)    );
        str.push_back("avg time: " + printTime(stat_avg_mks)    );
        str.push_back("exceed up threshold (" +
                printTime(stat_up_threshold_mks) +
                "): " +
                std::to_string(stat_up_thres_exceed_counter) +
                " times");

        if (isWithAllMeasurements)
        {
            str.push_back("all measurements:");
            for (size_t i = 0; i < stat_measurements_mks.size(); ++i) {
                str.push_back(printTime(stat_measurements_mks.at(i)));
            }
        }

        str.push_back(INDENTION_STRING);

        return str;
    }

    void print_statistics(bool isWithAllMeasurements = false)
    {
        print_statistics(form_stat_info(isWithAllMeasurements));
    }

    void print_statistics(std::vector<std::string> stat)
    {
        for (const auto& line : stat)
        {
            PRINT_LINE(line);
        }
    }

public:

    explicit CodeSpeed(std::string name)
        : name_(name){}

    ~CodeSpeed()
    {
        if (isMeasureStarted)
            stop_measure();
    }

    void initialize_two_point_measure(double up_threshold_mks = 0.0)
    {
        reset_stat(up_threshold_mks);
    }

    void first_point()
    {
        capture_start_time();
    }

    void second_point()
    {
        capture_stop_time_and_diff();
        calc_stat();
    }

    std::vector<std::string> get_statistics_info(bool isWithAllMeasurements = false)
    {
        return form_stat_info(isWithAllMeasurements);
    }


    void start_measure(double up_threshold_mks = 0.0)
    {
        reset_stat(up_threshold_mks);
        capture_start_time();
        isMeasureStarted = true;
    }

    void restart_measure(bool isPrintStat = false)
    {
        capture_stop_time_and_diff();
        calc_stat();

        if (isPrintStat)
        {
            print_statistics(true);
        }

        capture_start_time();
    }

    void stop_measure(bool isPrintInfo = true)
    {
        capture_stop_time_and_diff();

        if (isPrintInfo)
        {
            std::stringstream ss;
            ss << "Time measurement for " << name_ << " ";
            ss << printTime(lastMeasureMks);

            std::vector<std::string> v;
            v.push_back(INDENTION_STRING);
            v.push_back(ss.str());
            v.push_back(INDENTION_STRING);

            print_statistics(v);
        }
        isMeasureStarted = false;
    }

    double getLastMeasureMks() const
    {
        return lastMeasureMks;
    }

    std::string getLastMeasureDescription() const
    {
        return name_ + ": " +
                std::to_string(getLastMeasureMks()) + " mks";
    }
};


#ifndef DISABLE_CS_MEASURE_MACRO

#define START_CS_MEASURE(name) \
    CodeSpeed name(""#name""); \
    name.start_measure()

#define STOP_CS_MEASURE(name) \
    name.stop_measure()

#else

#define START_CS_MEASURE(name)

#define STOP_CS_MEASURE(name)

#endif


#define BENCHMARK_SPEED(name, counter, up_threshold_mks)\
    CodeSpeed name(""#name"");\
    name.start_measure(up_threshold_mks);\
    for(int i = 0; i < (counter); \
    i++, name.restart_measure(i == counter))
}

using namespace std;
using namespace Clipper2Lib;

void DoMyShapes();
void DoRabbit();
void DoSimpleShapes();
void System(const std::string& filename);


int main(int argc, char* argv[])
{
  DoMyShapes();
//  DoSimpleShapes();
//  DoRabbit();
//  //std::getchar();
}

void DoMyShapes()
{
    std::vector<double> linePoints;
//    linePoints.push_back({-0.5f, -0.5f});
//    linePoints.push_back({0.0f, 0.0f});
//    linePoints.push_back({+0.5f, -0.5f});


    float dt = 0.5;
    float t = -0.5f;
    float f = 0.05f;
    float A = 30.f;
    float width = 0.3f;
    for (int i = 0; i < 400; ++i)
    {
        linePoints.push_back(t * 1);
        linePoints.push_back(A * sin(2 * M_PI * f * t));
        t += dt;
    }

    // OPEN_PATHS SVG:

    PathsD op1, op2;
    FillRule fr2 = FillRule::EvenOdd;
    SvgWriter svg2;
    op1.push_back(MakePathD(linePoints));
    BENCHMARK_SPEED(inflate_sin, 100, 2500)
    {
        op2 = InflatePaths(op1, 5, JoinType::Miter, EndType::Round, 1.0);
    }
    SvgAddOpenSubject(svg2, op1, fr2, false);
    SvgAddSolution(svg2, op2, fr2, false);
    SvgAddCaption(svg2, "Miter Joins; Square Ends", 20, 210);

    SvgSaveToFile(svg2, "open_paths.svg", 800, 600, 20);

    System("open_paths.svg");

}

void DoSimpleShapes()
{
  // OPEN_PATHS SVG:

  PathsD op1, op2;
  FillRule fr2 = FillRule::EvenOdd;
  SvgWriter svg2;
  op1.push_back(MakePathD({ 80,60, 20,20, 180,20, 180,70, 25,150, 20,180, 180,180 }));
  op2 = InflatePaths(op1, 15, JoinType::Miter, EndType::Square, 3);
  SvgAddOpenSubject(svg2, op1, fr2, false);
  SvgAddSolution(svg2, op2, fr2, false);
  SvgAddCaption(svg2, "Miter Joins; Square Ends", 20, 210);
  op1 = TranslatePaths<double>(op1, 210, 0);
  op2 = InflatePaths(op1, 15, JoinType::Square, EndType::Square);
  SvgAddOpenSubject(svg2, op1, fr2, false);
  SvgAddSolution(svg2, op2, fr2, false);
  SvgAddCaption(svg2, "Square Joins; Square Ends", 230, 210);
  op1 = TranslatePaths<double>(op1, 210, 0);
  op2 = InflatePaths(op1, 15, JoinType::Bevel, EndType::Butt, 3);
  SvgAddOpenSubject(svg2, op1, fr2, false);
  SvgAddSolution(svg2, op2, fr2, false);
  SvgAddCaption(svg2, "Bevel Joins; Butt Ends", 440, 210);
  op1 = TranslatePaths<double>(op1, 210, 0);
  op2 = InflatePaths(op1, 15, JoinType::Round, EndType::Round);
  SvgAddOpenSubject(svg2, op1, fr2, false);
  SvgAddSolution(svg2, op2, fr2, false);
  SvgAddCaption(svg2, "Round Joins; Round Ends", 650, 210);
  SvgSaveToFile(svg2, "open_paths.svg", 800, 600, 20);
  System("open_paths.svg");

  // POLYGON JOINTYPES SVG:

  // 1. triangle offset - with large miter
  int err, scale = 100;
  PathsD p, solution;
  p.push_back(MakePathD({ 30,150, 60,350, 0,350 }));
  solution.insert(solution.end(), p.begin(), p.end());
  for (int i = 0; i < 5; ++i)
  {
    p = InflatePaths(p, 5, JoinType::Miter, EndType::Polygon, 10);
    solution.insert(solution.end(), p.begin(), p.end());
  }
  
  // 2. open rectangles offset bevelled, squared & rounded ...

  p.clear();
  p.push_back(MakePathD({ 100,30, 340,30, 340,230, 100,230 }));
  p.push_back(TranslatePath<double>(p[0], 60, 50));
  p.push_back(TranslatePath<double>(p[1], 60, 50));

  SvgWriter svg;
  SvgAddOpenSubject(svg, p);
  SvgAddCaption(svg, "Bevelled", 100, 15);
  SvgAddCaption(svg, "Squared", 160, 65);
  SvgAddCaption(svg, "Rounded", 220, 115);

  // nb: we must use the ClipperOffest class directly if we want to
  // perform different join types within the same offset operation
  // ClipperOffset only supports int64_t coords so, if we want better than unit
  // precision, we have to scale manually. (InflatePaths does this scaling internally)
  ClipperOffset co; 
  p = ScalePaths<double, double>(p, scale, err);
  // AddPaths - paths must be int64_t paths
  co.AddPath(TransformPath<int64_t, double>(p[0]), JoinType::Bevel, EndType::Joined);
  co.AddPath(TransformPath<int64_t, double>(p[1]), JoinType::Square, EndType::Joined);
  co.AddPath(TransformPath<int64_t, double>(p[2]), JoinType::Round, EndType::Joined);
  Paths64 sol64;
  co.Execute(scale * 10, sol64); // ClipperOffset solutions must be int64_t
  
  // de-scale and append to solution ...
  p = ScalePaths<double, int64_t>(sol64, 1.0 / scale, err);
  solution.insert(solution.end(), p.begin(), p.end());
  
  string filename = "polygon_jointypes.svg";
  SvgAddSolution(svg, solution, FillRule::EvenOdd, false);
  SvgSaveToFile(svg, filename, 800, 600, 20);
  System(filename);
}

void DoRabbit()
{
  SvgReader svg_reader;
  svg_reader.LoadFromFile("./rabbit.svg");
  PathsD p = svg_reader.GetPaths();

  JoinType jt = JoinType::Round;
  PathsD solution(p);

  while (p.size())
  {
    // nb: don't forget to scale the delta offset too!
    p = InflatePaths(p, -2.5, jt, EndType::Polygon);
    // SimplifyPaths (or RamerDouglasPeucker) is not 
    // essential but is highly recommended because it 
    // speeds up the loop and also tidies up the result
    p = SimplifyPaths(p, 0.25); // preferred over RDP()
    solution.reserve(solution.size() + p.size());
    copy(p.begin(), p.end(), back_inserter(solution));
  }

  FillRule fr = FillRule::EvenOdd;
  SvgWriter svg;
  SvgAddSolution(svg, solution, fr, false);
  SvgSaveToFile(svg, "solution_off2.svg", 450, 720, 0);
  System("solution_off2.svg");
}

void System(const std::string& filename)
{
#ifdef _WIN32
  system(filename.c_str());
#else
  system(("firefox " + filename).c_str());
#endif
}
