let selectedCell = null;

document.addEventListener('DOMContentLoaded', () => {
    const cells = document.querySelectorAll('.cell');
    const pickerCells = document.querySelectorAll('.picker-cell');

    // Select a cell on click (only mutable cells)
    cells.forEach(cell => {
        cell.addEventListener('click', () => {
            if (!cell.classList.contains('mutable')) return;

            if (selectedCell) {
                selectedCell.classList.remove('selected');
            }

            selectedCell = cell;
            cell.classList.add('selected');
        });
    });

    // Update selected cell when picker is clicked
    pickerCells.forEach(pickerCell => {
        pickerCell.addEventListener('click', () => {
            if (selectedCell && selectedCell.classList.contains('mutable')) {
                selectedCell.textContent = pickerCell.textContent;
            }
        });
    });
});