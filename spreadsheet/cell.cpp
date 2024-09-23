#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <stack>
#include <string>
#include <optional>
#include <memory>

// Реализуйте следующие методы
Cell::Cell(Sheet &sheet)
    : impl_(std::make_unique<EmptyImpl>()),
      sheet_(sheet)
{
}

Cell::~Cell() = default;

void Cell::Set(std::string text)
{
    std::unique_ptr<Impl> tmp;
    if (text.empty())
    {
        tmp = std::make_unique<EmptyImpl>();
    }
    else if (text.at(0) == FORMULA_SIGN && text.size() != 1)
    {
        tmp = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    }
    else
    {
        tmp = std::make_unique<TextImpl>(std::move(text));
    }
    if (HasCyclicDependencies(*tmp))
    {
        throw CircularDependencyException("Impossible to add a cell that causes a cyclic dependencies");
    }
    impl_ = std::move(tmp);
    

    // удаляем ссылку на данную ячейку у всех ячеек, на которые она ранее ссылалась
    for (auto referenced_cell : referenced_cells_)
    {
        referenced_cell->cells_reference_.erase(this);
    }

    // обновляем список ячеек, на которые теперь ссылается данная ячейка
    referenced_cells_.clear();
    
    for (auto &referenced_cell_pos : GetReferencedCells())
    {
        Cell* refrenced_cell = sheet_.GetConcreteCell(referenced_cell_pos);
        
        if (!refrenced_cell) {
            sheet_.SetCell(referenced_cell_pos, "");
            refrenced_cell = sheet_.GetConcreteCell(referenced_cell_pos);
            
        }
        
        referenced_cells_.insert(refrenced_cell);
        refrenced_cell->cells_reference_.insert(this);
    }

    InvalidateCache();
}

void Cell::Clear()
{
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const
{
    return impl_->GetValue();
}

std::string Cell::GetText() const
{
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const
{
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const
{
    return cells_reference_.empty();
}

Cell::Value Cell::EmptyImpl::GetValue() const
{
    return {};
}

std::string Cell::EmptyImpl::GetText() const
{
    return "";
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const
{
    return {};
}

bool Cell::EmptyImpl::IsCacheValid() const
{
    return true;
}

void Cell::EmptyImpl::InvalidateCache()
{
    return ;
}

Cell::TextImpl::TextImpl(std::string text)
    : text_(std::move(text))
{
}

Cell::Value Cell::TextImpl::GetValue() const
{
    if (text_.empty())
    {
        throw std::logic_error("There is no text");
    }
    if (text_.at(0) == ESCAPE_SIGN)
    {
        return text_.substr(1);
    }
    return text_;
}

std::string Cell::TextImpl::GetText() const
{
    return text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const
{
    return {};
}

bool Cell::TextImpl::IsCacheValid() const
{
    return true;
}

void Cell::TextImpl::InvalidateCache()
{
    return ;
}

Cell::FormulaImpl::FormulaImpl(std::string text, SheetInterface &sheet)
    : formula_(std::move(ParseFormula(text.substr(1)))),
      sheet(sheet)
{
          cache_ = std::nullopt;
}

Cell::Value Cell::FormulaImpl::GetValue() const
{
    FormulaInterface::Value value;
    if (!cache_.has_value())
    {
        value = formula_->Evaluate(sheet);
        cache_ = value;
    }
    else
    {
        value = cache_.value();
    }
    if (std::holds_alternative<double>(value))
    {
        return std::get<double>(value);
    }
    else
    {
        return std::get<FormulaError>(value);
    }
}

std::string Cell::FormulaImpl::GetText() const
{
    return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const
{
    return formula_->GetReferencedCells();
}

bool Cell::FormulaImpl::IsCacheValid() const
{
    return cache_.has_value();
}

void Cell::FormulaImpl::InvalidateCache()
{
    cache_.reset();
}

void Cell::InvalidateCache()
{
    if (!impl_->IsCacheValid())
    {
        return;
    }
    impl_->InvalidateCache();
    for (auto &cells_reference : cells_reference_)
    {
        cells_reference->InvalidateCache();
    }
}

bool Cell::HasCyclicDependencies(const Impl &cell) const
{
    if (cell.GetReferencedCells().empty())
    {
        return false;
    }

    /* реализуем нерекурсивный алгоритм DFS на стеке
    Шаг 1. Все вершины графа отмечаем, как не посещённые. Выбирается первая вершина и помечается как посещённая. Эту вершину кладем в контейнер — стек.
    Шаг 2. Пока стек не пустой:
        Извлекаем последнюю добавленную вершину.
        Просматриваем все смежные с ней не посещённые вершины и помещаем их в стек.
        Порядок выхода вершин из стека и будет порядком обхода вершин графа. */

    std::unordered_set<const Cell *> referenced_cells;
    for (const auto referenced_cell_pos : cell.GetReferencedCells())
    {
        referenced_cells.insert(sheet_.GetConcreteCell(referenced_cell_pos));
    }

    std::unordered_set<const Cell *> visited;
    std::stack<const Cell *> to_visit;

    to_visit.push(this);

    while (!to_visit.empty())
    {
        const Cell *visited_cell = to_visit.top();
        to_visit.pop();
        visited.insert(visited_cell);

        if (referenced_cells.find(visited_cell) != referenced_cells.end())
        {
            return true;
        }

        for (const Cell *next_referenced_cell : visited_cell->cells_reference_)
        {
            if (visited.find(next_referenced_cell) == visited.end())
            {
                to_visit.push(next_referenced_cell);
            }
        }
    }

    return false;
}