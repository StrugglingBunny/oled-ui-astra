//
// Created by Fir on 2024/1/21.
//

//每个列表应该有 1. 文本（需要显示的内容） 2. 跳转向量表（对应1，每一行都要跳转到哪里）
//其中文本以string形式 向量表以vector形式 以类的形式整合在一起 编写相应的构造函数 调用构造函数即是新建一个页面
//通过最后一项跳转向量表可以判断其类型 比如跳转至下一页 或者返回上一级 或者是复选框 或者是弹窗 或者是侧边栏弹窗 等等
//做一个astra类作为总的框架 在ui命名空间里

//分为
//驱动层 - 界面层（包括各种列表 磁铁 复选框 侧边栏 弹窗等） - 处理层（各种坐标的变换等）

//传进去的有三个东西 第一个是文字（要显示的东西） 第二个是类别（普通列表 跳转列表 复选框） 第三个是特征向量（指向复选框和弹窗对应的参数 要跳转的地方等）

////todo 看dreamView的代码 挨个检查每一部分是否都有对应
////todo 小组件下个版本更新

#include "item.h"

#include <utility>

/**
 *     ·  ·     ·   ·
 *  ·   ·  ·  ·   ·
 *  循此苦旅，直抵群星。
 *  ad astra per aspera.
 * happy Chinese new year!
 *      新年快乐！
 * ·   ·   ·      ·
 *   ·   ·    · ·   ·
 */

namespace astra {

void Item::updateConfig() {
  this->systemConfig = HAL::getSystemConfig();
  this->astraConfig = getUIConfig();
}

Camera::Camera() {
  this->xInit = 0;
  this->yInit = 0;

  this->x = 0;
  this->y = 0;
}

//这里的坐标应该都是负的 因为最终渲染的时候是加上摄像机的坐标
//所以说比如想显示下一页 应该是item本身的坐标减去摄像机的坐标 这样才会让item向上移动
//一个办法是用户传进来正的坐标 但是在摄像机内部 所有坐标都取其相反数 负的

//todo 如果出现问题 记得检查一下所有的坐标是不是都是有符号类型的 因为坐标全程都会出现负数
Camera::Camera(float _x, float _y) {
  this->xInit = 0 - _x;
  this->yInit = 0 - _y;

  this->x = 0 - _x;
  this->y = 0 - _y;
}

void Camera::go(float _x, float _y) {
  animation(&x, 0 - _x, astraConfig.cameraAnimationSpeed);
  animation(&y, 0 - _y, astraConfig.cameraAnimationSpeed);
}

void Camera::reset() {
  animation(&x, xInit, astraConfig.cameraAnimationSpeed);
  animation(&y, yInit, astraConfig.cameraAnimationSpeed);
}

void Camera::goDirect(float _x, float _y) {
  x = 0 - _x;
  y = 0 - _y;
}

void Camera::goNextPageItem() {
  animation(&y, y - systemConfig.screenHeight, astraConfig.cameraAnimationSpeed);
}

void Camera::goPreviewPageItem() {
  animation(&y, y + systemConfig.screenHeight, astraConfig.cameraAnimationSpeed);
}

void Camera::goNextTileItem() {
  animation(&x, x - (astraConfig.tilePicMargin + astraConfig.tilePicWidth), astraConfig.cameraAnimationSpeed);
}

void Camera::goPreviewTileItem() {
  animation(&x, x + (astraConfig.tilePicMargin + astraConfig.tilePicWidth), astraConfig.cameraAnimationSpeed);
}

Menu::Menu(std::string _title) {
  this->title = std::move(_title);
  this->selfType = LIST;
  this->childType = {};
  this->position.x = astraConfig.listTextMargin;
  this->position.y = 0;
  this->position.xTrg = astraConfig.listTextMargin;
  this->position.yTrg = 0;  //这里暂时无法计算trg 需要在addItem的时候计算 因为那时候才能拿到所有元素的数量
  this->selectIndex = 0;
  this->parent = nullptr;
  this->child.clear();
  this->pic.clear();
}

Menu::Menu(std::string _title, std::vector<uint8_t> _pic) {
  this->title = std::move(_title);
  this->pic = std::move(_pic);
  this->selfType = TILE;
  this->childType = {};
  this->position.x = 0;
  this->position.y = 0;
  this->position.xTrg = 0;  //这里暂时无法计算trg 需要在addItem的时候计算 因为那时候才能拿到所有元素的数量
  this->position.yTrg = astraConfig.tilePicTopMargin;
  this->selectIndex = 0;
  this->parent = nullptr;
  this->child.clear();
}

void Menu::init() {
  entryAnimation();

  if (selfType == TILE) {
    //受展开开关影响的坐标初始化
    if (astraConfig.tileUnfold) {
      for (auto _iter : child) _iter->position.x = 0 - astraConfig.tilePicWidth; //unfold from left.
      positionForeground.wBar = 0;  //bar unfold from left.

    } else {
      for (auto _iter : child) _iter->position.x = _iter->position.xTrg;
      positionForeground.wBar = positionForeground.wBarTrg;
    }

    //始终执行的坐标初始化
    //底部箭头和虚线的初始化
    positionForeground.yArrow = systemConfig.screenHeight;
    positionForeground.yDottedLine = systemConfig.screenHeight;

    //顶部进度条的从上方滑入的初始化
    positionForeground.yBar = 0 - astraConfig.tileBarHeight; //注意这里是坐标从屏幕外滑入 而不是height从0变大

  } else if (selfType == LIST) {
    //受展开开关影响的坐标初始化
    if (astraConfig.listUnfold) {
      for (auto _iter : child) _iter->position.y = 0; //text unfold from top.
      positionForeground.hBar = 0;  //bar unfold from top.
    } else {
      for (auto _iter : child) _iter->position.y = _iter->position.yTrg;
      positionForeground.hBar = positionForeground.hBarTrg;
    }

    //始终执行的坐标初始化
    positionForeground.xBar = systemConfig.screenWeight;
  }
}

void Menu::deInit() {
  //todo 未实现完全
  exitAnimation();
}

//todo 所有的跟文字有关的 包括selector 记住绘制文字时提供的坐标是文字左下角的坐标 改
void Menu::render(Camera* _camera) {
  //if (!isInit) init();

  if (selfType == TILE) {
    Item::updateConfig();

    HAL::setDrawType(1);

    //draw pic.
    for (auto _iter : child) {
       HAL::drawBMP(_iter->position.x + _camera->x, astraConfig.tilePicTopMargin + _camera->y, astraConfig.tilePicWidth, astraConfig.tilePicHeight, _iter->pic[0].data());
      //这里的xTrg在addItem的时候就已经确定了
      animation(&_iter->position.x, _iter->position.xTrg, astraConfig.tileAnimationSpeed);
    }

    //draw bar.
    //在屏幕最上方 两个像素高
    positionForeground.wBarTrg = systemConfig.screenWeight * ((selectIndex + 1) / getItemNum());
    HAL::drawBox(0, positionForeground.yBar, positionForeground.wBar, astraConfig.tileBarHeight);

    //draw left arrow.
    HAL::drawHLine(astraConfig.tileArrowMargin, positionForeground.yArrow, astraConfig.tileArrowWidth);
    HAL::drawPixel(astraConfig.tileArrowMargin + 1, positionForeground.yArrow + 1);
    HAL::drawPixel(astraConfig.tileArrowMargin + 2, positionForeground.yArrow + 2);
    HAL::drawPixel(astraConfig.tileArrowMargin + 1, positionForeground.yArrow - 1);
    HAL::drawPixel(astraConfig.tileArrowMargin + 2, positionForeground.yArrow - 2);

    //draw right arrow.
    HAL::drawHLine(systemConfig.screenWeight - astraConfig.tileArrowWidth - astraConfig.tileArrowMargin, positionForeground.yArrow, astraConfig.tileArrowWidth);
    HAL::drawPixel(systemConfig.screenWeight - astraConfig.tileArrowWidth, positionForeground.yArrow + 1);
    HAL::drawPixel(systemConfig.screenWeight - astraConfig.tileArrowWidth - 1, positionForeground.yArrow + 2);
    HAL::drawPixel(systemConfig.screenWeight - astraConfig.tileArrowWidth, positionForeground.yArrow - 1);
    HAL::drawPixel(systemConfig.screenWeight - astraConfig.tileArrowWidth - 1, positionForeground.yArrow - 2);

    //draw left button.
    HAL::drawHLine(astraConfig.tileBtnMargin, positionForeground.yArrow + 2, 9);
    HAL::drawBox(astraConfig.tileBtnMargin + 2, positionForeground.yArrow + 2 - 4, 5, 4);

    //draw right button.
    HAL::drawHLine(systemConfig.screenWeight - astraConfig.tileBtnMargin - 9, positionForeground.yArrow + 2, 9);
    HAL::drawBox(systemConfig.screenWeight - astraConfig.tileBtnMargin - 9 + 2, positionForeground.yArrow + 2 - 4, 5, 4);

    //draw dotted line.
    HAL::drawHDottedLine(0, (int16_t) positionForeground.yDottedLine, systemConfig.screenWeight);

    animation(&positionForeground.yDottedLine, positionForeground.yDottedLineTrg, astraConfig.tileAnimationSpeed);
    animation(&positionForeground.yArrow, positionForeground.yArrowTrg, astraConfig.tileAnimationSpeed);

    animation(&positionForeground.wBar, positionForeground.wBarTrg, astraConfig.tileAnimationSpeed);
    animation(&positionForeground.yBar, positionForeground.yBarTrg, astraConfig.tileAnimationSpeed);
  } else if (selfType == LIST) {
    Item::updateConfig();

    HAL::setDrawType(1);

    //allow x > screen height, y > screen weight.
    for (auto _iter : child) {
      HAL::drawChinese(_iter->position.x + _camera->x, _iter->position.y + astraConfig.listTextHeight + astraConfig.listTextMargin + _camera->y, _iter->title);
      //这里的yTrg在addItem的时候就已经确定了
      animation(&_iter->position.y, _iter->position.yTrg, astraConfig.listAnimationSpeed);
    }

    //draw bar.
    positionForeground.hBarTrg = systemConfig.screenHeight * ((float)(selectIndex + 1) / getItemNum());
    //HAL::drawHLine(systemConfig.screenWeight - astraConfig.listBarWeight, 0, astraConfig.listBarWeight);
    //HAL::drawHLine(systemConfig.screenWeight - astraConfig.listBarWeight, systemConfig.screenHeight - 1, astraConfig.listBarWeight);
    //HAL::drawVLine(systemConfig.screenWeight - ceil((float) astraConfig.listBarWeight / 2.0f), 0, systemConfig.screenHeight);
    HAL::drawBox(positionForeground.xBar, 0, astraConfig.listBarWeight, positionForeground.hBar);

    //light mode.
    if (astraConfig.lightMode) {
      HAL::setDrawType(2);
      HAL::drawBox(0, 0, systemConfig.screenWeight, systemConfig.screenHeight);
      HAL::setDrawType(1);
    }

    animation(&positionForeground.hBar, positionForeground.hBarTrg, astraConfig.listAnimationSpeed);
    animation(&positionForeground.xBar, positionForeground.xBarTrg, astraConfig.listAnimationSpeed);
  }
}

uint8_t Menu::getItemNum() const {
  return child.size();
}

Menu::Position Menu::getItemPosition(uint8_t _index) const {
  return child[_index]->position;
}

Menu *Menu::getNext() const {
  return child[selectIndex];
}

Menu *Menu::getPreview() const {
  return parent;
}

bool Menu::addItem(Menu *_page) {
  if (_page == nullptr) return false;
  else {
    //加入的第一个元素的类型决定了当前节点所有子元素的类型
    if (this->child.empty()) this->childType = _page->selfType;

    if (this->childType == _page->selfType) {
      _page->parent = this;
      this->child.push_back(_page);
      if (_page->selfType == LIST) {
        //_page->position.x = astraConfig.listTextMargin;
        _page->position.xTrg = astraConfig.listTextMargin;
        //_page->position.y = astraConfig.listTextMargin + this->getItemNum() * astraConfig.listLineHeight;
        _page->position.yTrg = (getItemNum() - 1) * astraConfig.listLineHeight;

        positionForeground.xBarTrg = systemConfig.screenWeight - astraConfig.listBarWeight;
      }
      if (_page->selfType == TILE) {
        //_page->position.x = astraConfig.tilePicMargin + this->getItemNum() * astraConfig.tilePicWidth;
        _page->position.xTrg = astraConfig.tilePicMargin + this->getItemNum() * astraConfig.tilePicWidth;
        //_page->position.y = astraConfig.tilePicTopMargin;
        _page->position.yTrg = astraConfig.tilePicTopMargin;

        _page->positionForeground.yBarTrg = 0;
        _page->positionForeground.yArrowTrg = systemConfig.screenHeight - astraConfig.tileArrowBottomMargin;
        _page->positionForeground.yDottedLineTrg = systemConfig.screenHeight - astraConfig.tileDottedLineBottomMargin;
      }
      return true;
    } else return false;
  }
}

void Selector::go(uint8_t _index) {
  Item::updateConfig();

  ////todo 在go的时候改变trg的值

  if (menu->selfType == Menu::TILE) {

    if (menu->selfType != menu->child[_index]->selfType) { /*todo 过渡动画 从大框到选择框*/ }

    xTrg = menu->child[_index]->position.xTrg - (astraConfig.tileSelectBoxWeight - astraConfig.tilePicWidth) / 2;
    yTrg = menu->child[_index]->position.yTrg - (astraConfig.tileSelectBoxHeight - astraConfig.tilePicHeight) / 2;

    yText = systemConfig.screenHeight; //给磁贴文字归零 从屏幕外滑入
    yTextTrg = systemConfig.screenHeight - astraConfig.tileTextBottomMargin;

  } else if (menu->selfType == Menu::LIST) {

    if (menu->selfType != menu->child[_index]->selfType) { /*todo 过渡动画 从选择框到大框*/ }

    xTrg = menu->child[_index]->position.xTrg - astraConfig.selectorMargin;
    yTrg = menu->child[_index]->position.yTrg ;
    wTrg = (float)HAL::getFontWidth(menu->child[_index]->title) + astraConfig.listTextMargin * 2;
  }
}

bool Selector::inject(Menu *_menu) {
  if (_menu == nullptr) return false;
  if (this->menu != nullptr) return false;
  this->menu = _menu;

  go(this->menu->selectIndex);  //注入之后要初始化选择框的位置

  return true;
}

bool Selector::destroy() {
  if (this->menu == nullptr) return false;

  delete this->menu;
  this->menu = nullptr;
}

void Selector::render(Camera* _camera) {
  Item::updateConfig();

  ////todo 未来可以做一个从磁贴大框向列表选择框的过渡动画 画的大框逐渐变小 最终变成选择框那么大 全过程都是没有R角 过渡完成后直接画R角的选择框即可
  ////todo 从列表到磁贴同理
  ////todo 判断依据 当目前进入的页面类型不等于前级页面类型时 执行过渡动画
  ////todo 如此这般的话 就需要增加两个坐标变量分别代表大框的长和宽 绘制时就不能直接从config中取了
  animation(&x, xTrg, astraConfig.selectorAnimationSpeed);
  animation(&y, yTrg, astraConfig.selectorAnimationSpeed);

  if (menu->selfType == Menu::TILE) {
    animation(&yText, yTextTrg, astraConfig.selectorAnimationSpeed);
    //animation(&wFrame, wFrameTrg, astraConfig.selectorAnimationSpeed);
    //animation(&hFrame, hFrameTrg, astraConfig.selectorAnimationSpeed);

    //draw text.
    //文字不受摄像机的影响
    HAL::setDrawType(1);
    HAL::drawChinese((systemConfig.screenWeight - (float)HAL::getFontWidth(menu->child[menu->selectIndex]->title)) / 2.0, yText - astraConfig.tileTitleHeight, menu->child[menu->selectIndex]->title);

    //draw box.
    //大框需要受摄像机的影响
    HAL::setDrawType(2);
    HAL::drawFrame(x + _camera->x, y + _camera->y, astraConfig.tileSelectBoxWeight, astraConfig.tileSelectBoxHeight);
    //HAL::drawFrame(x + _camera->x, y + _camera->y, wFrame, hFrame);

  } else if (menu->selfType == Menu::LIST) {
    animation(&w, wTrg, astraConfig.selectorWidthAnimationSpeed);
    //animation(&h, hTrg, astraConfig.selectorAnimationSpeed);

    //draw select box.
    //受摄像机的影响
    HAL::setDrawType(2);
    HAL::drawRBox(x + _camera->x, y + _camera->y, w, astraConfig.listLineHeight - 1, astraConfig.selectorRadius);
    //HAL::drawRBox(x, y, w, astraConfig.listLineHeight, astraConfig.selectorRadius);
    HAL::setDrawType(1);
  }
}
}
