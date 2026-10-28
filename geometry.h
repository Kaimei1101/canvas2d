#ifndef GEOMETRY_H
#define GEOMETRY_H

// 纯几何算法模块（实验二裁剪），不依赖 Qt/RenderTarget，保持低耦合
#include <algorithm>
#include <cmath>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "pixel_buffer.h"
#include "point.h"

namespace Geometry
{
// 叉积 (b-a) × (c-a)
double cross(Point a, Point b, Point c);

// 有符号面积的两倍；y 向下坐标系中 <0 对应屏幕逆时针绕序
double signedArea2(std::span<Point> p);

Point centroid(std::span<const Point> p);

// 点p到线段ab距离的平方
double pointToSegmentDistanceSqr(Point p, Point a, Point b);

// 凸多边形窗口增量构造（实验二-4 窗口确定）：
// pts 为固定绕序（signedArea2 <= 0，屏幕逆时针）的凸 n 边形环序顶点，
// 判定新点 p 是否“有且仅有对一条边与重心异侧”。
// 合法时返回该边的起点下标 i（新点应插入 pts[i] 与 pts[(i+1)%n] 之间），
// 否则返回 -1（点在内部/边界上，或对多条边异侧）。
int convexInsertEdge(const std::vector<Point> &pts, Point p);

// 由两个已确认点 + 新点生成固定绕序的三角形；共线返回 false
bool makeTriangle(Point a, Point b, Point p, std::vector<Point> &out);

// 线段 S->E 与直线 A->B 的交点参数 t（P = S + t·(E-S)）
double edgeIntersectParam(Point s, Point e, Point a, Point b);

// Sutherland-Hodgman：用凸裁剪窗口（屏幕逆时针绕序）裁剪多边形 subject
std::vector<Point> clipPolygon(std::vector<Point> subject,
                               std::span<const Point> clip);

// 整圆按 seg 段采样为多边形（裁剪时圆不再是圆，结果用多边形表示）
std::vector<Point> circleToPolygon(Point center, double radius, int seg = 64);

// 直线段对凸多边形窗口的裁剪（Liang-Barsky 凸多边形版）
// 窗口 clip 必须为屏幕逆时针绕序的凸多边形（内部满足 cross(a,b,p) <= 0）。
// 返回裁剪后的 (起,止)；整段在窗外时返回 std::nullopt。
std::optional<std::pair<Point, Point>>
clipLineSegment(Point s, Point e, std::span<const Point> clip);

// 圆弧对凸多边形窗口的裁剪
// 弧由 center/radius + 端点 ep1/ep2 + side 定义（与 Circle::ArcData 同语义：
// cross(ep2-ep1, p-ep1)*side > 0 的弧段被保留）。
// 返回裁剪后保留的若干段弧 (a,b)，方向与原弧一致（side 沿用原值）；
// 弧整段在窗外时返回空 vector。
struct ArcSeg {
    Point a;
    Point b;
    int side; // 与 Circle::ArcData::side 同语义：保留 cross(b-a, p-a)*side > 0
              // 的一侧
};

// 直线 a→b 与圆的交点相对圆心的极角（弧度），最多两个
std::vector<double> circleEdgeAngles(Point center, double radius, Point a,
                                     Point b);

std::vector<ArcSeg> clipArc(Point center, double radius, Point ep1, Point ep2,
                            int side, std::span<const Point> clip);

std::vector<Pixel> traceContour(const MaskBuffer &mask);
} // namespace Geometry

#endif // GEOMETRY_H
