#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Hash
{
public:
    size_t operator()(const Position pos) const
    {
        return std::hash<std::string>()(pos.ToString());
    }
};

class Sheet : public SheetInterface
{
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface *GetCell(Position pos) const override;
    CellInterface *GetCell(Position pos) override;

    const Cell *GetConcreteCell(Position pos) const;
    Cell *GetConcreteCell(Position pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream &output) const override;
    void PrintTexts(std::ostream &output) const override;

    // Можете дополнить ваш класс нужными полями и методами

private:
    void UpdatePrintableSize();
    std::unordered_map<Position, std::unique_ptr<Cell>, Hash> sheet_;
    Size printable_size_;
};