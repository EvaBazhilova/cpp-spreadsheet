#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <variant>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text)
{
    if (pos.IsValid())
    {
        if (sheet_.find(pos) == sheet_.end())
        {
            sheet_.emplace(pos, std::make_unique<Cell>(*this));
        }
        sheet_[pos]->Set(std::move(text));
        UpdatePrintableSize();
    }
    else
    {
        throw InvalidPositionException("Invalid set position");
    }
}

const CellInterface *Sheet::GetCell(Position pos) const
{
    if (pos.IsValid())
    {
        if (pos.row < printable_size_.rows && pos.col < printable_size_.cols)
        {
            if (sheet_.find(pos) != sheet_.end())
            {
                return sheet_.at(pos).get();
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        throw InvalidPositionException("Invalid get position");
    }
}

CellInterface *Sheet::GetCell(Position pos)
{
    if (pos.IsValid())
    {
        if (pos.row < printable_size_.rows && pos.col < printable_size_.cols)
        {
            if (sheet_.find(pos) != sheet_.end())
            {
                return sheet_.at(pos).get();
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        throw InvalidPositionException("Invalid get position");
    }
}

const Cell *Sheet::GetConcreteCell(Position pos) const
{
    if (pos.IsValid())
    {
            if (sheet_.find(pos) != sheet_.end())
            {
                return sheet_.at(pos).get();
            }
            else
            {
                return nullptr;
            }
    }
    else
    {
        throw InvalidPositionException("Invalid get position");
    }
}

Cell *Sheet::GetConcreteCell(Position pos)
{
    if (pos.IsValid())
    {
            if (sheet_.find(pos) != sheet_.end())
            {
                return sheet_.at(pos).get();
            }
            else
            {
                return nullptr;
            }
    }
    else
    {
        throw InvalidPositionException("Invalid get position");
    }
}

void Sheet::ClearCell(Position pos)
{
    if (pos.IsValid())
    {
        if (pos.row < printable_size_.rows && pos.col < printable_size_.cols)
        {
            if (sheet_.find(pos) != sheet_.end())
            {
                sheet_.at(pos)->Clear();
                UpdatePrintableSize();
            }
        }
    }
    else
    {
        throw InvalidPositionException("Invalid clear position");
    }
}

Size Sheet::GetPrintableSize() const
{
    return printable_size_;
}

void Sheet::PrintValues(std::ostream &output) const
{
    for (int row = 0; row < printable_size_.rows; row++)
    {
        for (int col = 0; col < printable_size_.cols; col++)
        {
            if (col > 0)
                output << "\t";
            const auto &it = sheet_.find({row, col});
            if (it != sheet_.end() && it->second != nullptr && !it->second->GetText().empty())
            {
                std::visit([&](const auto value)
                           { output << value; }, it->second->GetValue());
            }
        }
        output << "\n";
    }
}
void Sheet::PrintTexts(std::ostream &output) const
{
    for (int row = 0; row < printable_size_.rows; row++)
    {
        for (int col = 0; col < printable_size_.cols; col++)
        {
            if (col > 0)
                output << "\t";
            const auto &it = sheet_.find({row, col});
            if (it != sheet_.end() && it->second != nullptr && !it->second->GetText().empty())
            {
                output << it->second->GetText();
            }
        }
        output << "\n";
    }
}

std::unique_ptr<SheetInterface> CreateSheet()
{
    return std::make_unique<Sheet>();
}

void Sheet::UpdatePrintableSize()
{
    int max_row = 0;
    int max_col = 0;
    for (auto it = sheet_.begin(); it != sheet_.end(); it++)
    {
        if (it->second->GetText() != "")
        {
            max_row = std::max(max_row, it->first.row + 1);
            max_col = std::max(max_col, it->first.col + 1);
        }
    }
    printable_size_ = std::move(Size{max_row, max_col});
}