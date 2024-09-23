#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <unordered_set>
#include <vector>
#include <memory>

class Sheet;


class Cell : public CellInterface
{
public:
    Cell(Sheet &sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    class Impl
    {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        virtual void InvalidateCache() = 0;
        virtual bool IsCacheValid() const = 0;
        virtual ~Impl() = default;
    };
    class EmptyImpl : public Impl
    {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        bool IsCacheValid() const override;
        void InvalidateCache() override;
    };
    class TextImpl : public Impl
    {
    public:
        explicit TextImpl(std::string text);
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        bool IsCacheValid() const override;
        void InvalidateCache() override;

    private:
        std::string text_;
    };
    class FormulaImpl : public Impl
    {
    public:
        explicit FormulaImpl(std::string formula, SheetInterface &sheet);
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        bool IsCacheValid() const override;
        void InvalidateCache() override;

    private:
        std::unique_ptr<FormulaInterface> formula_;
        mutable std::optional<FormulaInterface::Value> cache_;
        const SheetInterface &sheet;
    };

    void InvalidateCache();
    bool HasCyclicDependencies(const Impl &cell) const;

    std::unique_ptr<Impl> impl_;
    Sheet &sheet_;
    std::unordered_set<Cell *> referenced_cells_; // на кого ссылается данная ячейка
    std::unordered_set<Cell *> cells_reference_; // кто ссылается на данную ячейку
};