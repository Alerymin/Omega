#include "go_to_parameter_controller.h"
#include "../apps_container.h"
#include <assert.h>

using namespace Shared;
using namespace Poincare;

namespace Regression {

GoToParameterController::GoToParameterController(Responder * parentResponder, Store * store, CurveViewCursor * cursor) :
  FloatParameterController(parentResponder, "Valider"),
  m_abscisseCell(PointerTableCellWithEditableText(&m_selectableTableView, this, m_draftTextBuffer)),
  m_store(store),
  m_cursor(cursor),
  m_xPrediction(true)
{
}

void GoToParameterController::setXPrediction(bool xPrediction) {
  m_xPrediction = xPrediction;
}

const char * GoToParameterController::title() const {
  if (m_xPrediction) {
    return "Prediction sachant x";
  }
  return "Prediction sachant y";
}

int GoToParameterController::numberOfRows() {
  return 2;
}

float GoToParameterController::previousParameterAtIndex(int index) {
  assert(index == 0);
  return m_previousParameter;
}

float GoToParameterController::parameterAtIndex(int index) {
  assert(index == 0);
  if (m_xPrediction) {
    return m_cursor->x();
  }
  return m_cursor->y();
}

void GoToParameterController::setParameterAtIndex(int parameterIndex, float f) {
  assert(parameterIndex == 0);
  if (m_xPrediction) {
    float y = m_store->yValueForXValue(f);
    m_store->centerAxisAround(CurveViewRange::Axis::X, f);
    m_store->centerAxisAround(CurveViewRange::Axis::Y, y);
    m_cursor->moveTo(f, y);
  } else {
    float x = m_store->xValueForYValue(f);
    if (isnan(x)) {
      return;
    }
    m_store->centerAxisAround(CurveViewRange::Axis::X, x);
    m_store->centerAxisAround(CurveViewRange::Axis::Y, f);
    m_cursor->moveTo(x, f);
  }
}

HighlightCell * GoToParameterController::reusableParameterCell(int index, int type) {
  assert(index == 0);
  return &m_abscisseCell;
}

int GoToParameterController::reusableParameterCellCount(int type) {
  return 1;
}

void GoToParameterController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  if (index == numberOfRows()-1) {
    return;
  }
  PointerTableCellWithEditableText * myCell = (PointerTableCellWithEditableText *) cell;
  if (m_xPrediction) {
    myCell->setText("x");
  } else {
    myCell->setText("y");
  }
  FloatParameterController::willDisplayCellForIndex(cell, index);
}

bool GoToParameterController::textFieldDidFinishEditing(TextField * textField, const char * text) {
  AppsContainer * appsContainer = ((TextFieldDelegateApp *)app())->container();
  Context * globalContext = appsContainer->globalContext();
  float floatBody = Expression::parse(text)->approximate(*globalContext);
  float parameter = m_store->yValueForXValue(floatBody);
  if (!m_xPrediction) {
    parameter = m_store->xValueForYValue(floatBody);
  }
  if (isnan(parameter)) {
    app()->displayWarning("Valeur non atteinte par la regression");
    return false;
  }
  return FloatParameterController::textFieldDidFinishEditing(textField, text);
}

void GoToParameterController::viewWillAppear() {
  m_previousParameter = parameterAtIndex(0);
  m_selectableTableView.reloadData();
}

void GoToParameterController::buttonAction() {
  StackViewController * stack = (StackViewController *)parentResponder();
  stack->pop();
  stack->pop();
}

}
