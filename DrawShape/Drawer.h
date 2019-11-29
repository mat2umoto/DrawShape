#pragma once

#include <vector>
#include <array>
#include <string>
#include <memory>
#include <limits>
#include <stdexcept>
#include <cmath>

// std::numeric_limits<T>::max, min を使用するため
#undef max
#undef min

namespace Drawer
{
// クラス前方宣言
class Manager;
class Canvas;
class Layer;
class Node;


// 座標データ構造体
template<typename T>
class Coord
{
public:
	// x座標
	T x;
	// y座標
	T y;

	// TODO: []演算子使う？使わなければクラスではなく構造体でもよい

	// []演算子（左辺値参照版）：x, yメンバを[0], [1]で参照可能にする
	T& operator[](std::size_t idx) &
	{
		if(idx == 0) return x;
		else if(idx == 1) return y;
		else throw std::out_of_range("Drawer::Coord::operator[] : index is out of range.");
	}
	// []演算子（const左辺値参照版）：x, yメンバを[0], [1]で参照可能にする
	const T& operator[](std::size_t idx) const &
	{
		if(idx == 0) return x;
		else if(idx == 1) return y;
		else throw std::out_of_range("Drawer::Coord::operator[] : index is out of range.");
	}
	// []演算子（const右辺値参照版）：x, yメンバを[0], [1]で参照可能にする
	T operator[](std::size_t idx) const &&
	{
		if(idx == 0) return x;
		else if(idx == 1) return y;
		else throw std::out_of_range("Drawer::Coord::operator[] : index is out of range.");
	}
};


// 最小包含箱クラス
template<typename T>
class BoundingBox
{
public:
	// 最小座標
	Coord<T> min;
	// 最大座標
	Coord<T> max;

	// コンストラクタ
	BoundingBox()
	{
		// 最小座標を型の最大値で初期化
		min.x = std::numeric_limits<T>::max();
		min.y = std::numeric_limits<T>::max();
		// 最大座標を型の最小値で初期化
		max.x = std::numeric_limits<T>::min();
		max.y = std::numeric_limits<T>::min();
	}

	// +=演算子：最小包含箱同士を合成
	BoundingBox& operator+=(const BoundingBox& rhs)
	{
		// より範囲の大きい矩形になるように合成する
		this->min.x = std::min(this->min.x, rhs.min.x);
		this->min.y = std::min(this->min.y, rhs.min.y);
		this->max.x = std::max(this->max.x, rhs.max.x);
		this->max.y = std::max(this->max.y, rhs.max.y);
		return *this;
	}
	// +=演算子：最小包含箱と座標値を合成
	BoundingBox& operator+=(const Coord<T>& rhs)
	{
		// より範囲の大きい矩形になるように合成する
		this->min.x = std::min(this->min.x, rhs.x);
		this->min.y = std::min(this->min.y, rhs.y);
		this->max.x = std::max(this->max.x, rhs.x);
		this->max.y = std::max(this->max.y, rhs.y);
		return *this;
	}

	// 幅を取得
	double GetWidth() const { return (max.x - min.x); }
	// 高さを取得
	double GetHeight() const { return (max.y - min.y); }

	// 有効確認
	bool Verify() const
	{
		if (GetWidth() >= 0.0 && GetHeight() > 0.0) return true;
		else return false;
	}
};

// +演算子：最小包含箱同士を合成
template<typename T>
BoundingBox<T> operator+(const BoundingBox<T>& lhs, const BoundingBox<T>& rhs) { return BoundingBox<T>(lhs) += rhs; }
// +演算子：最小包含箱と座標値を合成
template<typename T>
BoundingBox<T> operator+(const BoundingBox<T>& lhs, const Coord<T>& rhs) { return BoundingBox<T>(lhs) += rhs; }
template<typename T>
BoundingBox<T> operator+(const Coord<T>& lhs, const BoundingBox<T>& rhs) { return BoundingBox<T>(rhs) += lhs; }


// 描画キャンバスクラス
class Canvas
{
private:
	// 描画対象のデバイスコンテキスト
	CDC* m_pDC;

	// カレントペンオブジェクト
	CPen m_pen;
	// カレントブラシオブジェクト
	CBrush m_brush;

	// 拡大縮小率
	double m_ratio;
	// オフセット
	// ※コントロール座標だが計算制度のためにdouble型とする
	Coord<double> m_offset;

public:
	// 定数
	// 拡大縮小率の初期値
	static constexpr double DEFAULT_RATIO = 1.0;
	// オフセットの初期値
	static constexpr Coord<double> DEFAULT_OFFSET = { 0.0, 0.0 };
	// 円周率
	static constexpr double	PI = 3.141592653589793;
	// 距離の許容誤差
	static constexpr double LENGTH_TOLERANCE = 0.00001;
	// 角度の許容誤差(rad)
	static constexpr double ANGLE_TOLERANCE = 0.00001;
	// 最小グリッド描画サイズ(コントロール座標)
	static constexpr long DRAW_GRID_SIZE_MIN = 5;
	// 最小軸目盛描画サイズ(コントロール座標)
	static constexpr long DRAW_AXIS_SCALE_MIN = 10;
	// 目盛の長さ(軸の片側の長さ)
	static constexpr long AXIS_SCALE_LENGTH = 3;
	// 原点の中央矩形の大きさ
	static constexpr long ORIGIN_CENTER_SIZE = 2;
	// 点の強調時のサイズ
	static constexpr long LARGE_POINT_SIZE = 3;
	// 矢印の羽の軸からの角度(20°)
	static constexpr double ARROW_WING_ANGLE = 20.0 * PI / 180.0;

	// コンストラクタ
	Canvas(CDC* pDC);

	// 拡大縮小率
	void SetRatio(double val) { m_ratio = val;  }
	double GetRatio() const { return m_ratio; }
	// オフセット
	void SetOffset(Coord<double> val) { m_offset = val; }
	Coord<double> GetOffset() const { return m_offset; }

	// 座標系変換：内部キャンバス座標系→コントロール座標系
	Coord<long> CanvasToControl(const Coord<double> &canvasCoord) const;
	// 座標系変換：コントロール座標系→内部キャンバス座標系
	Coord<double> ControlToCanvas(const Coord<long>& ctrlCoord) const;
	BoundingBox<double> ControlToCanvas(const CRect& rect) const;

	// 背景を塗りつぶす
	void FillBackground(COLORREF color, const CRect& rect);
	// グリッド描画
	void DrawGrid(COLORREF color, double size, const CRect& rect);
	// 原点描画
	void DrawOrigin(COLORREF color, long size);
	// 軸描画
	void DrawAxis(COLORREF color, double scale, const CRect& rect);
	// 点を強調描画
	void DrawLargePoint(const Coord<double>& point);
	// 矢印先端描画
	void DrawArrowHead(const Coord<double>& start, const Coord<double>& end);
	// ベジエ曲線による円弧描画
	void DrawBezierArc();

	// 描画内容をファイル保存
	bool SaveBitmap(const std::string& filePath) const;
	// 描画内容をクリップボードへコピー
	bool CopyBitmap() const;

	// デバイスコンテキストを取得
	CDC* GetDC() const { return m_pDC; };
};


// ペン/ブラシ一時変更クラス
class PenBrushChanger
{
private:
	// 対象のデバイスコンテキスト
	CDC* m_pDC;

	// ペン
	CPen m_pen;
	// 変更前ペン
	CPen* m_pOldPen;

	// ブラシ
	CBrush m_brush;
	// 変更前ブラシ
	CBrush* m_pOldBrush;

public:
	// コンストラクタ：ペン変更
	PenBrushChanger(CDC* pDC, LOGPEN logPen) :
		m_pDC(pDC),
		m_pOldPen(nullptr),
		m_pOldBrush(nullptr)
	{
		if (m_pDC) {
			// ペン変更
			m_pen.CreatePenIndirect(&logPen);
			m_pOldPen = m_pDC->SelectObject(&m_pen);
		}
	}

	// コンストラクタ：ブラシ変更
	PenBrushChanger(CDC* pDC, LOGBRUSH logBrush) :
		m_pDC(pDC),
		m_pOldPen(nullptr),
		m_pOldBrush(nullptr)
	{
		if (m_pDC) {
			// ペン変更
			m_brush.CreateBrushIndirect(&logBrush);
			m_pOldBrush = m_pDC->SelectObject(&m_brush);
		}
	}

	// コピーコンストラクタ
	PenBrushChanger(const PenBrushChanger&) = delete;
	// コピー代入演算子
	PenBrushChanger& operator=(const PenBrushChanger&) = delete;

	// デストラクタ
	~PenBrushChanger()
	{
		if (m_pDC) {
			// ペンを元に戻す
			if (m_pOldPen) m_pDC->SelectObject(m_pOldPen);
			// ブラシを元に戻す
			if (m_pOldBrush) m_pDC->SelectObject(m_pOldBrush);
		}
	}
};


// ノードクラス
class Node
{
	// 座標データ配列型
	template<typename T, size_t N>
	using coords_t = std::array<Coord<T>, N>;

private:
	// 描画管理オブジェクト
	Manager& m_mng;

public:
	// コンストラクタ
	Node(Manager& mng) : m_mng(mng) {};
	// デストラクタ
	virtual ~Node() {};

	// 形状の最小包含箱を算出
	virtual BoundingBox<double> CalcBoundingBox() const = 0;
	// 形状が描画エリアに含まれるかチェック
	virtual bool IsIncludeDrawArea() const = 0;
	// 描画
	virtual void Draw() = 0;

};


// レイヤークラス
class Layer
{
private:
	// 描画管理オブジェクト
	Manager& m_mng;
	// ノードコレクション
	std::vector<std::unique_ptr<Node>> m_nodes;

	// 描画フラグ
	bool m_isDraw;

public:
	// コンストラクタ
	Layer(Manager& mng);

	// 初期化
	void Clear();

	// 全形状の最小包含箱を算出
	BoundingBox<double> CalcBoundingBox() const;

	// 描画フラグ
	void SetIsDraw(bool val) { m_isDraw = val; }
	bool GetIsDraw() const { return m_isDraw; }

	// 描画
	void Draw();
};


// 描画管理クラス
class Manager
{
private:
	// 上位コントロール
	COleControl* m_pCtrl;
	// 描画キャンバス
	Canvas m_canvas;

	// ベースレイヤー
	Layer m_baseLayer;
	// 描画レイヤーコレクション
	std::vector<std::unique_ptr<Layer>> m_layers;

	// 背景色
	COLORREF m_backColor;
	// グリッド色
	COLORREF m_gridColor;
	// グリッドサイズ
	double m_gridSize;
	// 原点色
	COLORREF m_originColor;
	// 原点サイズ
	long m_originSize;
	// 軸色
	COLORREF m_axisColor;
	// 軸スケール
	double m_axisScale;
	// グリッド描画可否
	bool m_isDrawGrid;
	// 原点描画可否
	bool m_isDrawOrigin;
	// 軸描画可否
	bool m_isDrawAxis;
	// 矢印描画可否
	bool m_isDrawArrow;
	// 円中心点描画可否
	bool m_isDrawCenter;

	// カレントレイヤー番号
	int m_currentLayerNo;

	// 全形状の最小包含箱を算出
	BoundingBox<double> CalcBoundingBox() const;

public:
	// 定数
	// 最大拡大率
	static constexpr double RATIO_MAX = 50000.0;
	// 最小縮小率
	static constexpr double RATIO_MIN = 0.0001;
	// 最大移動量X,Y
	static constexpr double OFFSET_MAX = 999999.0;

	// コンストラクタ
	Manager(COleControl* pCtrl, CDC* pDC);

	// コピーコンストラクタ
	Manager(const Manager&) = delete;
	// 代入演算子
	Manager& operator=(const Manager&) = delete;

	// 背景色
	void SetBackColor(COLORREF val) { m_backColor = val; };
	COLORREF GetBackColor() { return m_backColor; };
	// グリッド色
	void SetGridColor(COLORREF val) { m_gridColor = val; };
	COLORREF GetGridColor() { return m_gridColor; };
	// グリッドサイズ
	void SetGridSize(double val) { m_gridSize = val; };
	double GetGridSize() { return m_gridSize; };
	// 原点色
	void SetOriginColor(COLORREF val) { m_originColor = val; };
	COLORREF GetOriginColor() { return m_originColor; };
	// 原点サイズ
	void SetOriginSize(long val) { m_originSize = val; };
	long GetOriginSize() { return m_originSize; };
	// 軸色
	void SetAxisColor(COLORREF val) { m_axisColor = val; };
	COLORREF GetAxisColor() { return m_axisColor; };
	// 軸スケール
	void SetAxisScale(double val) { m_axisScale = val; };
	double GetAxisScale() { return m_axisScale; };
	// グリッド描画可否
	void SetIsDrawGrid(bool val) { m_isDrawGrid = val; };
	bool GetIsDrawGrid() { return m_isDrawGrid; };
	// 原点描画可否
	void SetIsDrawOrigin(bool val) { m_isDrawOrigin = val; };
	bool GetIsDrawOrigin() { return m_isDrawOrigin; };
	// 軸描画可否
	void SetIsDrawAxis(bool val) { m_isDrawAxis = val; };
	bool GetIsDrawAxis() { return m_isDrawAxis; };
	// 矢印描画可否
	void SetIsDrawArrow(bool val) { m_isDrawArrow = val; };
	bool GetIsDrawArrow() { return m_isDrawArrow; };
	// 円中心点描画可否
	void SetIsDrawCenter(bool val) { m_isDrawCenter = val; };
	bool GetIsDrawCenter() { return m_isDrawCenter; };

	// カレントレイヤー番号
	void SetCurrentLayerNo(int val) { m_currentLayerNo = val; };
	int GetCurrentLayerNo() { return m_currentLayerNo; };

	// 初期化
	void Clear();

	// 描画
	void Draw();
	// 描画(デザインモード用)
	void DrawDesignMode(const CRect& rect);

	// 拡大縮小
	bool Zoom(double coef, const Coord<long>& base);
	// 拡大縮小（カーソル位置基準）
	bool Zoom(double coef);
	// パン
	bool Pan(const Coord<long>& move);
	// フィット
	void Fit(double shapeOccupancy);

	// レイヤー枚数を取得
	size_t GetLayerCount() const { return m_layers.size(); }

	// 上位コントロールの矩形を取得（クライアント座標）
	CRect GetControlRect() const;
};


//// 線分ノードクラス
//class LineNode : public Node
//{
//private:
//	// 座標データ配列
//	coords_t<double, 2> m_datas;
//
//public:
//	// コンストラクタ
//	LineNode(Manager& mng);
//
//	// 形状の最小包含箱を算出
//	BoundingBox<double> CalcBoundingBox() const override;
//	// 形状が描画エリアに含まれるかチェック
//	bool IsIncludeDrawArea() const override;
//
//	// 描画
//	void Draw() override;
//};

}	// namespace DrawShapeLib