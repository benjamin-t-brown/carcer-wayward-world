import { useState } from 'react';

export const useReRender = () => {
  const [render, setRender] = useState(false);
  return () => {
    setRender(!render);
  };
};
