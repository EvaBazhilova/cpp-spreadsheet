#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <functional>
#include <sstream>
#include <vector>

using namespace std::literals;

namespace
{
    class Formula : public FormulaInterface
    {
    public:
        // Реализуйте следующие методы:
        explicit Formula(std::string expression)
        try: ast_(ParseFormulaAST(expression))
        {
        }
        catch(...)
        {
        throw FormulaException("Impossible to set incorrect formula");
        }

        Value Evaluate(const SheetInterface &sheet) const override
        {
                const std::function<double(Position)> args = [&sheet](const Position pos) -> double
                {
                    if (!pos.IsValid())
                    {
                        throw FormulaError(FormulaError::Category::Ref);
                    }
                    if (sheet.GetCell(pos)==nullptr)
                    {
                        return 0.0;
                    }
               
                    CellInterface::Value value = sheet.GetCell(pos)->GetValue();
                    if (std::holds_alternative<double>(value))
                    {
                        return std::get<double>(value);
                    }
                    if (std::holds_alternative<std::string>(value))
                    {
                            double number = 0.0;
                            if (!std::get<std::string>(value).empty()) {
                                std::istringstream in(std::get<std::string>(value));
                                if (!(in >> number) || !in.eof()) {throw FormulaError(FormulaError::Category::Value);}
                            }
                            return number;
                    }
                    throw std::get<FormulaError>(value);
                };
                try
            {
                return ast_.Execute(args);
            }
            catch (const FormulaError &evaluate_error)
            {
                return evaluate_error;
            }
        }
        
        std::string GetExpression() const override
        {
            std::ostringstream output;
            ast_.PrintFormula(output);

            return output.str();
        }

        std::vector<Position> GetReferencedCells() const override
        {
            std::vector<Position> ref_cells;
            for (const auto ref_cell : ast_.GetCells())
            {
                if (ref_cell.IsValid() && std::find(ref_cells.begin(), ref_cells.end(), ref_cell) == ref_cells.end())
                {
                    ref_cells.push_back(ref_cell);
                }
            }
            return ref_cells;
        }

    private:
        FormulaAST ast_;
    };
} // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression)
{
    return std::make_unique<Formula>(std::move(expression));
}